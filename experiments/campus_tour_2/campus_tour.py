

filename = "imp_exp.txt"
lat_long_array = []

with open(filename, "r") as txtfile:
	for line in txtfile:
		if 't=0' in line: 
			continue
		if 'lat=' in line and 'long=' in line:
			parts = line.split()
			lat_str = parts[1]
			long_str = parts[2]
			latitude = float(lat_str.replace('lat=', '').replace(',', ''))
			longitude = float(long_str.replace('long=', ''))
			lat_long_array.append((latitude,longitude))
			#print(f"{latitude:.4f},{longitude:.4f},")
#print(lat_long_array)

latitudes = [lat for lat, _ in lat_long_array]
longitudes = [lon for _, lon in lat_long_array]


#print(latitudes)



import csv
import numpy as np
import cartopy.crs as ccrs
import matplotlib.pyplot as plt
import cartopy.io.img_tiles as cimgt
from cartopy.mpl.ticker import LongitudeFormatter, LatitudeFormatter
import io,time
from urllib.request import urlopen, Request
from PIL import Image
plt.ion()

def image_spoof(self, tile): # this function pretends not to be a Python script
    url = self._image_url(tile) # get the url of the street map API
    req = Request(url) # start request
    req.add_header('User-agent','Anaconda 3') # add user agent to request
    fh = urlopen(req) 
    im_data = io.BytesIO(fh.read()) # get image
    fh.close() # close url
    img = Image.open(im_data) # open image with PIL
    img = img.convert(self.desired_tile_form) # set image format
    return img, self.tileextent(tile), 'lower' # reformat for cartopy

################################
# parsing the GPS coordinates
################################
#

#######################################
# Formatting the Cartopy plot
#######################################
#
plotname = "Campus Tour"
cimgt.GoogleTiles.get_image = image_spoof # reformat web request for street map spoofing
osm_img = cimgt.GoogleTiles() # spoofed, downloaded street map

fig = plt.figure(figsize=(14,12),facecolor='#FCFCFC') # open matplotlib figure
ax1 = plt.axes(projection=osm_img.crs) # project using coordinate reference system (CRS) of street map
ax1.set_title(plotname,fontsize=16)
lat_zoom = 0.001 # zoom out from the bounds of lats
lon_zoom = 0.005 # zoom out from the bounds of lons
extent = [np.min(longitudes)-lon_zoom,np.max(longitudes)+lon_zoom,np.min(latitudes)-lat_zoom,np.max(latitudes)+lat_zoom] # map view bounds
ax1.set_extent(extent) # set extents
ax1.set_xticks(np.linspace(extent[0],extent[1],7),crs=ccrs.PlateCarree()) # set longitude indicators
ax1.set_yticks(np.linspace(extent[2],extent[3],7)[1:],crs=ccrs.PlateCarree()) # set latitude indicators
lon_formatter = LongitudeFormatter(number_format='0.1f',degree_symbol='',dateline_direction_label=True) # format lons
lat_formatter = LatitudeFormatter(number_format='0.1f',degree_symbol='') # format lats
ax1.xaxis.set_major_formatter(lon_formatter) # set lons
ax1.yaxis.set_major_formatter(lat_formatter) # set lats
ax1.xaxis.set_tick_params(labelsize=14)
ax1.yaxis.set_tick_params(labelsize=14)

scale = np.ceil(-np.sqrt(2)*np.log(np.divide((extent[1]-extent[0])/2.0,350.0))) # empirical solve for scale based on zoom
scale = (scale<20) and scale or 19 # scale cannot be larger than 19
ax1.add_image(osm_img, int(scale+1)) # add OSM with zoom specification

#######################################
# Plot the GPS points
#######################################
#

# Function to convert a Matplotlib figure to a PIL Image
from PIL import Image
def fig_to_image(fig, first_frame_size=None, index=0):
    buf = io.BytesIO()
    fig.savefig(buf, format='png', bbox_inches='tight')
    buf.seek(0)
    img = Image.open(buf).copy()
    current_frame_size = img.size
    #print(f"Frame size: {current_frame_size}")
    buf.close()
    
    return img

images = []

for ii in range(0,len(longitudes),2):
    ax1.plot(longitudes[ii],latitudes[ii], markersize=1,marker='o',linestyle='',color='#b30909',transform=ccrs.PlateCarree(),label='GPS Point') # plot points
    transform = ccrs.PlateCarree()._as_mpl_transform(ax1) # set transform for annotations

    plt.pause(0.001) # pause between point plots
    images.append(fig_to_image(fig))
    
    

width, height = images[0].size
base_image = Image.new('RGB', (width, height), 'white')


base_image.save("gps1.gif", save_all=True,append_images=images[1:], duration=100,loop=0)
"""

# Create a list to hold the centered images
centered_images = []
prev_size = 0
curr_size = 0
change_index = 1
# Center each image on the base image
for index, img in enumerate(images,start=1):
    # Create a new blank image with the same size as the base image
    #print(f"Frame size: {img.size}")
    curr_size = img.size[0]
    #print("curr_size:	",curr_size)
    
    if curr_size != prev_size:
    	change_index = index
    	prev_size = curr_size
    new_img = Image.new("RGB", (width, height), "white")
    
    # Calculate the position to paste the smaller image onto the base image
    left = (width - img.size[0]) // 2
    top = (height - img.size[1]) // 2
    
    # Paste the smaller image onto the new image
    new_img.paste(img, (left, top))
    
    # Append the new image to the centered images list
    centered_images.append(new_img)

# Save the GIF with the centered images
print(change_index)
base_image.save("gps.gif", save_all=True, append_images=centered_images[change_index-1:], duration=50, loop=0)
"""

