
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D, art3d
from matplotlib.patches import Polygon
import mpl_toolkits.mplot3d.art3d as art3d
import numpy as np
import sys


#______________________________________________________________________________

class Ctel_drawer:
	def __init__(self):
		self.keplerOutputFile = 'KeplerViewerData.dat'
		# self.htlow = 1854400
		# self.htup = 1876500

		hCentre = 1.7947e+06
		hWindow = 10

		self.htlow = hCentre - hWindow
		self.htup = hCentre + hWindow

		# self.htlow = 1873000
		# self.htup = 1875000

		# self.htlow = 100000
		# self.htup = 120000

		self.xshift = 0
		self.xlow = 0.0
		self.xup = 0.0
		self.zs = []
		self.minToT=-1
		self.maxToT=-1

		# Plot esthetics.
		self.fig = plt.figure(figsize = [15,8])
		self.ax = self.fig.gca(projection='3d')
		self.ax._axis3don = False 
		plt.subplots_adjust(left=0.03, right=0.97, top=0.95, bottom=0.05)


		# Plot object options.
		self.drawing_chips = True
		self.drawing_pixels = True
		self.drawing_dead_pixels = False
		self.drawing_clusters = True
		self.drawing_tracks = True
		self.drawing_vols = False
		self.drawing_hough_states = False
		self.drawing_vertices = True
		self.drawing_central_regions = True





#______________________________________________________________________________

	def draw(self):
		# Loop over the output file.
		if self.drawing_pixels:
			self.scanPixelToTs()
			self.m = cm.ScalarMappable(cmap=cm.gist_rainbow)
			self.m.set_array([self.minToT, self.maxToT])
			self.m.set_clim(vmin=self.minToT, vmax=self.maxToT)
			#self.m.set_label('ToT')
			cb = self.fig.colorbar(self.m, fraction=0.046, pad=0.01, shrink = 0.6)
			cb.set_label('ToT')


		f = open(self.keplerOutputFile, 'r')
		for line in f: 
			if line[0] != "#": self.draw_object(line)		
		f = open(self.keplerOutputFile, 'r')
		for line in f:
			if len(line.split()) == 0: continue
			if line[0] != "#" and line.split()[0] == "SqTrackArea" and self.drawing_vols: 
				self.draw_sq_track_area(line.split())
		f = open(self.keplerOutputFile, 'r')
		for line in f: 
			if len(line.split()) == 0: continue
			if line[0] != "#" and line.split()[0] == "Cluster" and self.drawing_clusters: 
				self.draw_cluster(line.split())

		f = 0.4
		xmid = (self.xup + self.xlow)/2.0
		xdif = self.xup - self.xlow
		self.ax.set_xlim3d(-f*xdif + xmid, xmid + f*xdif)
		self.ax.set_ylim3d(-6, 30)
		self.ax.set_zlim3d(-6, 20)

		# plt.scatter([0], [-6], [-6])
		# plt.scatter([0], [20], [20])
		self.ax.set_zlabel('y (mm)', fontsize=20)
		self.ax.set_ylabel('x (mm)', fontsize=20)
		self.ax.set_xlabel('z (mm)', fontsize=20)
		plt.tight_layout()
		plt.show()


#______________________________________________________________________________

	def scanPixelToTs(self):
		f = open(self.keplerOutputFile, 'r')
		for line in f: 
			if len(line.split()) == 0: continue
			if line[0] != "#" and line.split()[0] == "Pixel" and self.htlow < float(line.split()[13]) < self.htup:

				ToT = float(line.split()[14])
				if self.minToT == -1:
					self.minToT = ToT
					self.maxToT = ToT

				elif ToT < self.minToT: 
					self.minToT = ToT
				elif ToT > self.maxToT: 
					self.maxToT = ToT


#______________________________________________________________________________

	def draw_object(self, line):
		line_bits = line.split()
		if len(line_bits) != 0:
			if line_bits[0] == "Chip" and self.drawing_chips == True: self.draw_chip(line_bits)
			elif line_bits[0] == "Pixel" and self.drawing_pixels == True: self.draw_pixel(line_bits)
			elif line_bits[0] == "DeadPixel" and self.drawing_dead_pixels == True: self.draw_dead_pixel(line_bits)
			elif line_bits[0] == "Track" and self.drawing_tracks == True: self.draw_track(line_bits)
			elif line_bits[0] == "SqTrackArea" and self.drawing_vols == True: self.draw_sq_track_area(line_bits)
			elif line_bits[0] == "PatternRecogCircle" and self.drawing_vols == True: self.draw_PatternRecogCircle(line_bits)
			elif line_bits[0] == "HoughState" and self.drawing_hough_states == True: self.draw_hough_state(line_bits)
			elif line_bits[0] == "Vertex" and self.drawing_vertices == True: self.draw_vertex(line_bits)
			elif line_bits[0] == "CentralRegion" and self.drawing_central_regions: self.draw_central_region(line_bits)





#______________________________________________________________________________

	def draw_central_region(self, line_bits):
		zs = 5*[float(line_bits[-1])]
		xlow = float(line_bits[1])
		xup = float(line_bits[2])
		ylow = float(line_bits[3])
		yup = float(line_bits[4])

		xs = [xlow, xlow, xup, xup, xlow]
		ys = [ylow, yup, yup, ylow, ylow]

		plt.plot(zs, xs, ys, c = 'k')



#______________________________________________________________________________

	def draw_PatternRecogCircle(self, line_bits):
		x = float(line_bits[1])
		y = float(line_bits[2])
		z = float(line_bits[3])
		t = float(line_bits[4])
		r = float(line_bits[5])
		if self.htlow < t < self.htup:
			xs = []
			ys = []
			zs = []
			for theta in np.linspace(0, 2*np.pi, 100):
				xs.append(x + r*np.cos(theta))
				ys.append(y + r*np.sin(theta))
				zs.append(z);

			plt.plot(zs, xs, ys, c = '0.5')




#______________________________________________________________________________

	def draw_chip(self, line_bits):
		corners = np.empty([4, 3])
		for i in range(4):
			corners[i][1] = float(line_bits[3*i+1])
			corners[i][2] = float(line_bits[3*i+1+1])
			corners[i][0] = float(line_bits[3*i+1+2]) + self.xshift
			if corners[i][0] < self.xlow: self.xlow = corners[i][0]
			if corners[i][0] > self.xup: self.xup = corners[i][0] 

		plt.plot(corners.T[0], corners.T[1], corners.T[2], alpha = 0.4, c = 'k')
		plt.plot([corners.T[0][0], corners.T[0][3]], 
			[corners.T[1][0], corners.T[1][3]], 
			[corners.T[2][0], corners.T[2][3]], alpha = 0.4, c = 'k')

		self.zs.append(np.average(corners.T[0]))

		plane = art3d.Poly3DCollection([corners], alpha = 0.06)
		plane.set_color('0.5')
		self.ax.add_collection3d(plane)





#______________________________________________________________________________

	def draw_cluster(self, line_bits):
		if self.htlow < float(line_bits[4]) < self.htup:
			x = [float(line_bits[3]) + self.xshift]
			z = [float(line_bits[2])]
			y = [float(line_bits[1])]
			size = 35

			col = 'g'
			shape = 'o'
			if int(line_bits[5]) == 2: 
				col = 'b'
				shape = '^'
			if int(line_bits[5]) == 3: 
				col = 'b'
				shape = '*'
				size = 60
			if int(line_bits[5]) == 5: 
				col = '#6666FF'
				shape = '*'
				size = 60
			if int(line_bits[5]) == 4: 
				col = '#6666FF'
				shape = '>'
			if int(line_bits[5]) == 1: 
				col = 'k'
				shape = 'v'

			if not self.drawing_pixels: self.ax.scatter(x, y, z, s=size, c=col, lw = 0, alpha = 1.0, marker = shape, zorder=4)
			else: 
				self.ax.scatter([x[0]+0.35], y, z, s=size, c=col, lw = 0, alpha = 1.0, marker = shape, zorder=4)
				self.ax.scatter([x[0]-0.35], y, z, s=size, c=col, lw = 0, alpha = 1.0, marker = shape, zorder=4)






#______________________________________________________________________________

	def draw_dead_pixel(self, line_bits):
		corners = np.empty([4, 3])
		for i in range(4):
			corners[i][1] = float(line_bits[3*i+1])
			corners[i][2] = float(line_bits[3*i+1+1])
			corners[i][0] = float(line_bits[3*i+1+2]) + self.xshift

		plane = art3d.Poly3DCollection([corners], alpha = 1.0, linewidths=0.5, zorder=-1)
		plane.set_color('k')
		plane.set_edgecolor('k')
		self.ax.add_collection3d(plane)


#______________________________________________________________________________

	def draw_pixel(self, line_bits):
		if self.htlow < float(line_bits[13]) < self.htup:
			corners = np.empty([4, 3])
			for i in range(4):
				corners[i][1] = float(line_bits[3*i+1])
				corners[i][2] = float(line_bits[3*i+1+1])
				corners[i][0] = float(line_bits[3*i+1+2]) + self.xshift

			plane = art3d.Poly3DCollection([corners], alpha = 1.0, linewidths=0.5, zorder=-1)
			plane.set_color(self.m.to_rgba(float(line_bits[14])))
			plane.set_edgecolor('k')
			self.ax.add_collection3d(plane)




#______________________________________________________________________________

	def draw_hough_state(self, line_bits):
		if self.htlow < float(line_bits[7]) < self.htup and self.htlow < float(line_bits[8]) < self.htup:
			xs = [float(line_bits[3]), float(line_bits[6])]
			zs = [float(line_bits[2]), float(line_bits[5])]
			ys = [float(line_bits[1]), float(line_bits[4])]

			plt.plot(xs, ys, zs, c='b', alpha = 1, lw = 0.8)






#______________________________________________________________________________

	def draw_track(self, line_bits):
		if self.htlow < float(line_bits[5]) < self.htup:
			mz = float(line_bits[1])
			cz = float(line_bits[2])
			my = float(line_bits[3])
			cy = float(line_bits[4])

			xs = [self.xlow, self.xup]
			zs = [my*xs[0] + cy, my*xs[1] + cy]
			ys = [mz*xs[0] + cz, mz*xs[1] + cz]

			plt.plot(xs, ys, zs, c='k', alpha = 0.6, lw = 0.6)



#______________________________________________________________________________

	def draw_vertex(self, line_bits):
		if self.htlow < float(line_bits[4]) < self.htup:
			x = float(line_bits[3])
			for i in range(int(line_bits[5])):
				mz = float(line_bits[6+i*5])
				cz = float(line_bits[7+i*5])
				my = float(line_bits[8+i*5])
				cy = float(line_bits[9+i*5])

				col='k'

				if int(line_bits[10+i*5]) == 1: 
					xs = [self.xlow, x]
					col='b'
				else: xs = [x, self.xup]
				
				zs = [my*xs[0] + cy, my*xs[1] + cy]
				ys = [mz*xs[0] + cz, mz*xs[1] + cz]

				print xs, ys, zs

				plt.plot(xs, ys, zs, c=col, alpha = 0.8, lw = 0.9, linestyle = '--')





#______________________________________________________________________________

	# def draw_sq_track_area(self, line_bits):
		# Add square on each chip for the intercept.
		# volTlow = float(line_bits[1])
		# volTup = float(line_bits[2])

		# if volTup > self.htlow and volTlow < self.htup:
		# 	corners = np.empty([4, 3])
		# 	for i in range(4):
		# 		corners[i][1] = float(line_bits[3*i+3])
		# 		corners[i][2] = float(line_bits[3*i+3+1])
		# 		corners[i][0] = float(line_bits[3*i+3+2]) + self.xshift

		# 	plt.plot(corners.T[0], corners.T[1], corners.T[2], alpha = 0.3, c = 'k')
		# 	plt.plot([corners.T[0][0], corners.T[0][3]], 
		# 		[corners.T[1][0], corners.T[1][3]], 
		# 		[corners.T[2][0], corners.T[2][3]], alpha = 0.3, c = 'k')

	def draw_sq_track_area(self, line_bits):
		# Add lines along the corners, and intermediate contours.
		# Four corners first.
		volTlow = float(line_bits[8])
		volTup = float(line_bits[9])

		if volTup > self.htlow and volTlow < self.htup:
			col = '0.1'
			m_lw = 0.9

			mz = float(line_bits[1])
			cz = float(line_bits[2])
			my = float(line_bits[3])
			cy = float(line_bits[4])

			x = float(line_bits[7])
			y = float(line_bits[6])
			z = float(line_bits[5])

			xs = [self.xlow, x, self.xup]

			dl = abs(self.xlow - x)
			du = abs(self.xup - x)

			zs0 = [dl*my+y+cy, y+cy, du*my+y+cy]
			zs1 = [dl*my+y+cy, y+cy, du*my+y+cy]
			zs2 = [-dl*my+y-cy, y-cy, -du*my+y-cy]
			zs3 = [-dl*my+y-cy, y-cy, -du*my+y-cy]

			ys0 = [dl*mz+z+cz, z+cz, du*mz+z+cz]
			ys1 = [-dl*mz+z-cz, z-cz, -du*mz+z-cz]
			ys2 = [-dl*mz+z-cz, z-cz, -du*mz+z-cz]
			ys3 = [dl*mz+z+cz, z+cz, du*mz+z+cz]

			# plt.plot(xs, ys0, zs0, lw= m_lw, c = col)
			# plt.plot(xs, ys1, zs1, lw= m_lw, c = col)
			# plt.plot(xs, ys2, zs2, lw= m_lw, c = col)
			# plt.plot(xs, ys3, zs3, lw= m_lw, c = col)

			for ix in self.zs:
				dx = abs(ix - x)
				zs = [dx*my+y+cy, dx*my+y+cy, -dx*my+y-cy, -dx*my+y-cy, dx*my+y+cy]
				ys = [dx*mz+z+cz, -dx*mz+z-cz, -dx*mz+z-cz, dx*mz+z+cz, dx*mz+z+cz]
				plt.plot(5*[ix], ys, zs, lw= 0.5*m_lw, c = col)



#______________________________________________________________________________

my_drawer = Ctel_drawer()
my_drawer.draw()
