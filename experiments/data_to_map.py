"""

# csv->google maps
# Author: Yiming Alan Li (yimili@student.ethz.ch)
# Date: 09.11.2023
# Description: plots lat/lon data stored in csv onto google maps 

    data string has the following format:
    Date [mm/dd/yyyy], Time [HH:MM:SS.ZZ], Latitude [deg], Longitude [deg]

"""
import gmplot

# Create the map plotter:
apikey = '' # (your API key here)
gmap = gmplot.GoogleMapPlotter(47.40864480072206, 8.506195396058997, 18, apikey=apikey)

import csv


filename = 'exp02.csv'
lat_long_array = []

with open(filename,'r') as csvfile:
    csv_reader = csv.reader(csvfile)
    # Skip the header row
    next(csv_reader)


    # Loop through the rows in the csv reader
    for row in csv_reader:
        #print(row)
        # The latitude and longitude are at index 2 and 3 respectively
        latitude = float(row[2])
        longitude = float(row[3])
    
        # Append as tuples to the array
        lat_long_array.append((latitude, longitude))


path = zip(*lat_long_array)
gmap.plot(*path, edge_width=3, color='red')
gmap.draw('map2.html')