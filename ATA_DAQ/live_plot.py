import csv
import matplotlib.pyplot as plt
from matplotlib import style
import time
import math
import threading
from Queue import Queue
import os
from datetime import datetime

from memory_profiler import profile

stopGraphing = False

# Generator which returns new lines as they're added.
# It is easy to change the refresh rate because this just changes
# how often new lines are checked for
# https://stackoverflow.com/questions/5419888/reading-from-a-frequently-updated-file
def follow(fname, sleep_sec=1):
    global stopGraphing
    with open(fname, "r") as current:
        #current = open(fname, "r")
        curino = os.fstat(current.fileno()).st_ino
        while not stopGraphing:
            while True:
                line = current.readline()
                if not line:
                    break
                yield line

            try:
                if os.stat(fname).st_ino != curino:
                    new = open(fname, "r")
                    current.close()
                    current = new
                    curino = os.fstat(current.fileno()).st_ino
                    continue
            except IOError:
                pass
            time.sleep(sleep_sec)
        yield None

# Function for seperate thread of reading from file
def read_thread(fname, q):
    for l in follow(fname):
        # Check whether to stop reading
        if l is None:
            break
        # Put the line into the queue
        q.put(l)
    
style.use('ggplot')
class LiveGrapher:
    def __init__(self, fname, col_names, col_step, max_points=1000, truncate=True, subsample=False):
        self.fig = None
        
        self.fname = fname
        self.col_names = col_names
        self.col_step = col_step
        
        self.truncate = truncate
        self.subsample = subsample
        self.max_points = max_points
        
        # Count of how many frames have been plotted
        self.frame_count = 0
        self.max_frames = 60
        
        # Queue for getting data from file
        self.q = Queue(maxsize=0)
        self.thread = threading.Thread(target=read_thread, args=(self.fname,self.q))
        self.thread.setDaemon(True)
        self.thread.start()
        
        self.x = []
        self.y = []
        for i in range(len(self.col_names)-1):
            self.y.append([])
        
        self.lastFilePosition = None
    
    def __del__(self):
        self.update()
        self.q.join()
        self.thread.join()
    
    # Function to parse a returned line from a file
    def parseLine(self, line):
        # Split on comma
        line = line.split(",")
        try:
            # Add sample number
            self.x.append(int(line[0]))
            for col in range(len(self.y)):
                # If the value is not a float or doesn't exist, skip
                try:
                    self.y[col].append(float(line[col*self.col_step+2]))
                except:
                    pass
        except:
            # We were part of the header or had some other issue
            print("Could not parse line correctly")
            print("***********")
            print(line)
            print("***********")
    
    # Save figure information
    def set_figure(self, rows, cols):
        self.fig_rows = rows
        self.fig_cols = cols
    
    # Create all of the subplots for the figure
    def create_plots(self, fig_rows=0, fig_cols=0):
        self.fig = plt.figure()
        plt.ion()
        plt.show()
        
        if fig_rows == 0:
            fig_rows = self.fig_rows
        if fig_cols == 0:
            fig_cols = self.fig_cols
        # Create subplots
        self.axs = []
        for row in range(fig_rows):
            for col in range(fig_cols):
                self.axs.append(self.fig.add_subplot(fig_rows, fig_cols, row*fig_cols + col + 1))
    
    # Read the source file and create the arrays which will be plotted
    #@profile
    def read_file(self):
        # Sample number
        self.x = []
        # Y columns
        self.y = []
        for i in range(len(self.col_names)-1):
            self.y.append([])
        # Read file
        #with open(self.fname) as f:
        if self.fin is not None:
            reader = csv.DictReader(self.fin)
            for row in reader:
                # Add sample number
                self.x.append(int(row['Sample']))
                for i in range(len(self.col_names)-1):
                    # If the value is not a float or doesn't exist, skip
                    try:
                        self.y[i].append(float(row[self.col_names[i+1]]))
                    except:
                        pass
            # Save the last position in the file to simplify getting new
            # data
            self.lastFilePosition = self.fin.tell()

    # Update the data and plot
    def update(self):
        # Iterate over the queue info
        while not self.q.empty():
            self.parseLine(self.q.get())
            self.q.task_done()
        
        # Check whether we've plotted too many frames
        self.frame_count += 1
        if (self.frame_count > self.max_frames):
            self.frame_count = 0
            plt.close(self.fig)
            self.fig = None
        
        if self.fig is None:
            self.create_plots()
            
        self.plot()

    # Get the Y limits for the graphs
    def get_y_limits(self):
        y_mins = []
        y_maxs = []
        
        for i in self.y:
            if len(i) != 0:
                y_mins.append(min(i))
                y_maxs.append(max(i))
        
        self.y_min = min(y_mins)
        self.y_max = max(y_maxs)
        
        # Round to nearest 2, making sure the range is at least 2
        self.y_max = int(math.ceil(self.y_max/2.0)) * 2
        self.y_min = int(math.floor(self.y_min/2.0)) * 2

    # Update all plots
    def plot(self):
        # Clear plots
        for ax in self.axs:
            ax.clear()
        
        # Plot the new data
        #print(len(self.y))
        for i in range(len(self.axs)):
            y_len = len(self.y[i])
            
            min_x = 0
            max_x = y_len
            step = 1

            if (y_len >= self.max_points):
                # Truncate graph display to last N samples
                if self.truncate:
                    min_x = y_len - self.max_points
                    max_x = y_len
            
                # Subsample history for display
                if self.subsample:
                    step  = y_len / self.max_points
                    min_x = y_len % self.max_points
                    max_x = y_len
            
            # Make sure both arrays are the same size
            self.axs[i].plot(self.x[min_x:max_x:step], self.y[i][min_x:max_x:step], label=self.col_names[i+1])
            
        # Update the y axis
        self.get_y_limits()
        
        for ax in self.axs:
            ax.set_ylim(self.y_min, self.y_max)
        
        plt.pause(0.01)

class VoltageGraph(LiveGrapher):
    # Create a live view of the voltage readings
    def __init__(self, fname, update_delay, truncate=True, subsample=False):
        col_names = ['Sample', 'Channel 0','Channel 1','Channel 2',
                     'Channel 3','Channel 4','Channel 5','Channel 6',
                     'Channel 7','Channel 8','Channel 9','Channel 10',
                     'Channel 11','Channel 12','Channel 13','Channel 14',
                     'Channel 15']
        LiveGrapher.__init__(self, fname, col_names, 1, truncate=truncate, subsample=subsample)
        # Not actually used
        self.update_delay = update_delay
        
        # Create the plots
        self.set_figure(4, 4)
        
        # Read the file
        #self.read_file()

class ThermoGraph(LiveGrapher):
    # Create a live view of the voltage readings
    def __init__(self, fname, update_delay, truncate=True, subsample=False):
        col_names = ['Sample', 'Channel 0','Channel 1','Channel 2',
                     'Channel 3','Channel 4','Channel 5','Channel 6',
                     'Channel 7','Channel 8','Channel 9','Channel 10',
                     'Channel 11']
        LiveGrapher.__init__(self, fname, col_names, 2, truncate=truncate, subsample=subsample)
        # Not actually used
        self.update_delay = update_delay
        
        # Create the plots
        self.set_figure(3, 4)
        
        # Read the file
        #self.read_file()

class CryoGraph(LiveGrapher):
    # Create a live view of the thermo readings from the cryo sensors
    def __init__(self, fname, update_delay, truncate=True, subsample=False):
        col_names = ['Sample','SPI0','SPI1','SPI2','SPI3']
        LiveGrapher.__init__(self, fname, col_names, 1, truncate=truncate, subsample=subsample)
        
        # Create the plots
        self.set_figure(2,2)
        
        # Read the file
        #self.read_file()

v_graph = VoltageGraph("voltages_slow.csv", 10,subsample=True)
#t_graph = ThermoGraph("thermo_slow.csv", 10,subsample=True)
c_graph = CryoGraph("../pyBusPirateLite/SPI_logging.csv",10,subsample=True)

# Get time of start of the program
start_time = time.time()
# Check if program was running for 2 hours.
# If so, kill it to prevent data acquisition from crashing
#while time.time() - start_time < 10000:
while time.time() - start_time < 100:
    print("Updating at "+str(datetime.now().strftime("%H:%M:%S")))
    v_graph.update()
    #t_graph.update()
    c_graph.update()
    
    # How many seconds between graph updates
    time.sleep(5)

print("Stopping graph")
stopGraphing = True
del v_graph
del c_graph
"""
print(t_graph.x)
print(t_graph.y)
t_graph.update()
raw_input("Press enter")
"""
