import urllib.request
import matplotlib.pyplot as plt
import numpy as np

first_tanker_array = np.empty(1)
second_tanker_array = np.empty(1)
while True:
    contents = urllib.request.urlopen("http://192.168.1.100").read().split()
    first_sensor_level = contents[1][:-1]
    first_tanker_array=np.append(first_tanker_array,first_sensor_level)
    second_sensor_level = contents[4][:-1]
    second_tanker_array=np.append(second_tanker_array,second_sensor_level)
    plt.plot(second_tanker_array)
    plt.draw()
    plt.pause(0.1)
    plt.clf()
