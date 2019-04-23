import numpy as np
import cv2
import argparse

pixel = []
world = []

def DLT(xz, uv):

    xz = np.asarray(xz)
    uv = np.asarray(uv)

    n = xz.shape[0]
    A = []

    for i in range(n):
        x, z = xz[i, 0], xz[i, 1]
        u, v = uv[i, 0], uv[i, 1]
        A.append( [0, 0, 0, -x, -z, -1, v * x, v * z, v] )
        A.append( [x, z, 1, 0, 0, 0, -u * x, -u * z, -u] )
    
    A = np.asarray(A) 

    # Find the 11 parameters:
    U, S, V = np.linalg.svd(A)

    # The parameters are in the last line of Vh and normalize them
    L = V[-1, :] / V[-1, -1]
    # Camera projection matrix
    H = L.reshape(3, 3)
    HI = np.linalg.inv(H)
    HI = HI / HI[2, 2]

    xz2 = np.dot( HI, np.concatenate((uv.T, np.ones((1, uv.shape[0]))) ) ) 
    xz2 = xz2 / xz2[2, :] 
    # Mean distance:
    err = np.sqrt( np.mean(np.sum( (xz2[0:2, :].T - xz)**2, 1)) ) 

    return HI, err

def call(xz, uv):
    P, err = DLT(xz, uv)
    print(P, err)

    xz2 = np.dot(P, [1012, 428, 1]) 
    print(xz2 / xz2[2])

def pick(event,x,y,flags,param):
    global mouseX,mouseY
    if event == cv2.EVENT_LBUTTONDBLCLK:
        mouseX, mouseY = int(x) * 2, int(y) * 2
        print(mouseX, mouseY, input[mouseY, mouseX])
        pixel.append([mouseX, mouseY])
        wX = float(raw_input("World X: "))
        world.append([wX, input[mouseY, mouseX]])

parser = argparse.ArgumentParser()
parser.add_argument("--im", help = "Image file path")
parser.add_argument("--depth", help = "Depth file path")
args = parser.parse_args()

img = cv2.imread(args.im, 1)
img = cv2.resize(img, (1024, 512)) 
cv2.namedWindow('image')
cv2.setMouseCallback('image', pick) 

input = np.array(np.loadtxt(args.depth, skiprows = 0)).astype(float)

while(1):
    cv2.imshow('image', img)
    k = cv2.waitKey(0) & 0xFF
    if k == 27:
        break
call(world, pixel)