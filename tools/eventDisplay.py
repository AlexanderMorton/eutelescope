#!/usr/bin/env python2
import matplotlib as mpl
import matplotlib.pyplot as plt
import csv
import re
import sys, getopt
#If we want to combine events.
isCom=False
#Upper lower limits to plot
eventLow=0
eventHigh=10000000000
isTrackView=False
inputfileHist=None
def getRowsTracks(reader):
    event = []
    posX = []
    posY = []
    posZ = []
    track = []
    loc = []
    for row in reader:
     #   print "row : ",row
        try: 
            #if eventLow < float(row[0])  < eventHigh:#Only collect a sub set.
            event.append(float(row[0]))
            track.append(float(row[1]))
            loc.append(float(row[2]))
            posX.append(float(row[3]))
            posY.append(float(row[4]))
            posZ.append(float(row[5]))
        except:
            print "fail to parse line: ", row
    return event,track,loc,posX,posY,posZ

def getRowsHits(reader):
    event = []
    posX = []
    posY = []
    posZ = []
    for row in reader:
      #  print "row : ",row
        try: 
      #      print "row : ",row , " " , eventLow , " ", eventHigh, " ",float(row[0])
            #if eventLow < float(row[0])  < eventHigh:#Only collect a sub set.
       #     print "pass"
            event.append(float(row[0]))
            posX.append(float(row[1]))
            posY.append(float(row[2]))
            posZ.append(float(row[3]))
        except:
            print "fail to parse line: ", row
    return event,posX,posY,posZ


def plotTracks(xx,zt):
    lastZ=zt[-1]
    xS=[]
    yS=[]
    for x,z in zip(xx,zt):#Draw all tracks with different line. The list give each state from (0->6+DUT)
        xS.append(z)
        yS.append(x)
        if lastZ == z:#If find last plane hit then add segments to histogram.
      #      print "xS ", xS
       #     print "yS ", yS
            for i in range(0,len(xS)-1):
        #        print "i : " , i
                xP=[xS[i],xS[i+1]]
                yP=[yS[i],yS[i+1]]
         #       print "xp ", xP
          #      print "yp ", yP
                plt.plot(xP, yP, 'g', mew=3,label="Track")
            xS=[]
            yS=[]
def XYPlot(evtt,tt,loc,xxt,zt,xxh,zh,ext):
    fig = plt.figure()
    if isCom:
        fig.suptitle("Events %i %i " % (eventLow,eventHigh))
    else:
        fig.suptitle("Events %i " % (evtt[0]))
    if isTrackView:
        plotSensors(loc,xxt,zt)    
    else:
        plotSensors(loc,xxh,zh)    

    ###Plot tracks
    plotTracks(xxt,zt)#Split by z displacement
    ###Now the hits.
    plt.plot(zh, xxh, 'rx',markersize=15,mew=2,label="Hits" )
    #Fix the x axis.
    plt.xlim(min(zt)-100,max(zt)+100)#Have to fix this after plotting.
    if isTrackView:
     #   print "track view ", min(xxt), " ",max(xxt)
        per = 0.05*(max(xxt)- min(xxt))
        plt.ylim(min(xxt)-per,max(xxt)+per)#Have to fix this after plotting.
    else:
        per = 0.05*(max(xxh)- min(xxh))
        plt.ylim(min(xxt)-per,max(xxt)+per)#Have to fix this after plotting.

    if isTrackView:
        view = "track"
    else:
        view = "hits"
    if isCom:
        event = "eventsFrom"  + str(int(eventLow)).replace(".0","") + "to" +str(int(eventHigh)).replace(".0","") + ext + "_" +view + ".png"
    else:
        event = inputfileHist + "/event"  + str(evtt[0]).replace(".0","")+ext +"_"+view+ ".png"
    fig.savefig(event, bbox_inches='tight')
    #print "Save ", event

def runFigureMakingForInputs(hitsInfo,trackInfo):
    #print "hitInfo ",hitsInfo
    if not isCom:
        evth,xh,yh,zh = zip(*hitsInfo)
        evtt,tt,loc,xt,yt,zt = zip(*trackInfo)
    else:
        evth,xh,yh,zh = hitsInfo
        evtt,tt,loc,xt,yt,zt = trackInfo

    #print "plot making..."
    ##The same sensor should be placed on the same plots.
    XYPlot(evtt,tt,loc,xt,zt,xh,zh,"X")
    XYPlot(evtt,tt,loc,yt,zt,yh,zh,"Y")

def f7(seq):
    seen = set()
    seen_add = seen.add
    return [x for x in seq if not (x in seen or seen_add(x))]
         
def plotSensors(loc,xx,zt):
    #print "locB" , loc
    uniZ=list(set(zt))#Convert to set to get repeated Z then back to list to get iterable.
    loc=f7(loc)
    uniZ=sorted(uniZ)
    #print " loc/uni " , loc, uniZ
    for l,z in zip(loc,uniZ):
      #  print "z:",z
        xPoints = [z,z]
        yPoints = [min(xx),max(xx)]
     #   print "loc ", l
        if l < 6:
            plt.plot(xPoints, yPoints, 'b',mew=6,label="Sensors")
        else:
            plt.plot(xPoints, yPoints, 'k',mew=6,label="Sensors")
def splitByEvent(info):
    ##print"info: ", info
    infoS = []
    evt= []
    ePrev=info[0][0]
    #print "ePrev",ePrev
    for fo in zip(*info): 
    #    print "event", fo, "   ",fo[0]
        if ePrev < fo[0]:#If new event then add to last one and start again.
            import numpy as np
            evt.append(infoS)
   #         print "evt2 ", evt, " ",ePrev 
            infoS = []
            ePrev=fo[0]
        infoS.append(fo)
  #  print "evt ", evt
    return evt
try:
    opts, args = getopt.getopt(sys.argv[1:],"i:t:o:l:h:")
except getopt.GetoptError:
  #  print 'eventDisplayt.py -ih <Hits> -it <tracks>'
    sys.exit(2)
#print opts
for opt, arg in opts:
    if opt in ("-i"):
        inputfileHits = arg
   #     print "found, ", inputfileHits
    elif opt in ("-t"):
        inputfileTracks = arg
   #     print "input: ", inputfileTracks
    elif opt in ("-o"):
        inputfileHist = arg
    elif opt in ("-l"):
        eventLow=arg
    elif opt in ("-h"):
        eventHigh=arg


#Range decided with events passed.
with open(inputfileHits, 'rb') as f:
    reader = csv.reader(f)
    hitInfo = getRowsHits(reader)
with open(inputfileTracks, 'rb') as f:
    reader = csv.reader(f)
    trackInfo = getRowsTracks(reader)
#Create different list for events if no combination
if isCom:
    #print "Combine"
    runFigureMakingForInputs(hitInfo,trackInfo)
else:
    hitInfo = splitByEvent(hitInfo)
    trackInfo = splitByEvent(trackInfo)
    numEvt=0
    for eh,et in zip(hitInfo,trackInfo):#A different list for each event.
     #   print "here"
      #  print "eh ",eh , " et ", et
        if numEvt < 10:
            runFigureMakingForInputs(eh,et)
        numEvt=numEvt+1

