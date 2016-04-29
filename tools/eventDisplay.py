#!/usr/bin/env python2
import matplotlib as mpl
import matplotlib.pyplot as plt
import csv
import re
import sys, getopt
from scipy.stats import chi2
from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})
#If we want to combine events.
#Upper lower limits to plot
def getRowsTracks(reader):
    """
    Will take a series of rows and create a series of columns for tracks csv. 
    """
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
            event.append(float(row["Event"]))
            track.append(float(row["TrackNum"]))
            loc.append(float(row["StateLocation"]))
            posX.append(float(row["X"]))
            posY.append(float(row["Y"]))
            posZ.append(float(row["Z"]))
        except:
            print "fail to parse line: ", row
    return [event,track,loc,posX,posY,posZ]

def getRowsHits(reader):
    """
    Will take a series of rows and create a series of columns for hits csv. 
    """

    event = []
    posX = []
    posY = []
    posZ = []
    for row in reader:
        try: 
            event.append(float(row["Event"]))
            posX.append(float(row["X"]))
            posY.append(float(row["Y"]))
            posZ.append(float(row["Z"]))
        except:
            print "fail to parse line: ", row
    return [event,posX,posY,posZ]
def plotSegments(sub,xS,yS):
    for i in range(0,len(xS)-1):
        xP=[xS[i],xS[i+1]]
        yP=[yS[i],yS[i+1]]
        sub.plot(xP, yP, 'g', mew=3,label="Track")


def plotTracks(sub,xx,zt):
    """
    Will link kinked tracks between hits on planes. Will link all hits until we go backwards in space.
    @param xx A list of the x/y postions.
    @param zt A list of z positions which must match the x/y positions given.
    @return trackCount The number of tracks plotted.
    """
    lastZ=0
    xS=[]
    yS=[]
    trackCount=0 
    sub.plot(zt, xx, 'gx',markersize=15,mew=2,label="Track Hits" )
    for x,z in zip(xx,zt):#Draw all tracks with different line. The list give each state from (0->6+DUT)
        if  z - lastZ < 0:#Found complete track since moved back in space. 
            #print z," ", x
            trackCount = trackCount +1
            plotSegments(sub,xS,yS)
            xS=[]
            yS=[]
        xS.append(z)
        yS.append(x)
        lastZ=z
    plotSegments(sub,xS,yS)#Once more for the last track. 
    return trackCount

def XYPlot(sub,evtt,tt,loc,xxt,zt,xxh,zh,isTrackView,ext):
    """
    Will plot the X/Y figures. The plot either shows all hits or just the track region.
    @param isTrackView This specifies if only the sensors with track hits should bne included.
    @param ext This is either X or Y to specify the direction
    @return sc,tc Sensor/track number
    """
    #fig.suptitle("Events %i " % (evtt[0]))
    #fig.tight_layout()
    #if isTrackView:
    #    sc=plotSensors(loc,xxt,zt)    
    #else:
    sc=plotSensors(sub,loc,xxh,zh)    
    ###Now the hits.
    sub.plot(zh, xxh, 'rx',markersize=15,mew=2,label="None Track Hits" )
    ###Plot tracks
    tc=plotTracks(sub,xxt,zt)#Split by z displacement
    #Fix the x axis.
    sub.set_xlim(min(zt)-100,max(zt)+100)
    #Fix the Y axis
    if isTrackView:
        per = 0.2*(max(xxt)- min(xxt))
        sub.set_ylim(min(xxt)-per,max(xxt)+per)
    else:
        print "Hits to find min"
        per = 0.2*(max(xxh)- min(xxh))
        sub.set_ylim(min(xxh)-per,max(xxh)+per)
    sub.set_ylabel("Sensor (" + ext + ") mm" )
    return sc,tc

def runFigureMakingForInputs(hitsInfo,trackInfo,isTrackView):
    """
    Unpack both the X/Y axis info and then plot
    """
    #print hitsInfo[0]
    evth=hitsInfo[0];xh=hitsInfo[1];yh=hitsInfo[2];zh=hitsInfo[3]
    evtt=trackInfo[0];tt=trackInfo[1];loc=trackInfo[2];xt=trackInfo[3];yt=trackInfo[4];zt=trackInfo[5]
    fig, (ax1, ax2) = plt.subplots(3, sharex=True)
    fig.subplots_adjust(hspace=0)
    plt.title("Event %i" % (evtt[0]))
    plt.setp([a.get_xticklabels() for a in f.axes[:-1]], visible=False)
    #fig.set_size_inches(5,5)
    scX,tcX=XYPlot(ax1,evtt,tt,loc,xt,zt,xh,zh,isTrackView,"X")
    scY,tcY=XYPlot(ax2,evtt,tt,loc,yt,zt,yh,zh,isTrackView,"Y")
    ax.set_xlabel("Beam line (Z) mm" )
    if isTrackView:
        view = "track"
    else:
        view = "hits"
    event = inputfileHist + "/event"  + str(evtt[0]).replace(".0","") +"_"+view+ ".png"
    fig.savefig(event)

    if tcX == tcY and scX == scY :
        return scX,tcX
    else:
        print "Track/Sensor number for x/y plots do not match"
        raise
def f7(seq):
    seen = set()
    seen_add = seen.add
    return [x for x in seq if not (x in seen or seen_add(x))]
         
def plotSensors(sub,loc,xx,zt):
    """
    Uses all the unique Z position to determine sensor position. Assume location come in the same order.  The loop through location and z postions.
    The input his can be any series.
    @param loc Vector of locations. Can be a repeated list
    @param xx A series of hits to determine the min/max of the sensor.
    @param zt The z position which will be sorted to remove repeated positions. MUST be same order as location.
    @return senCou Sensor count.
    """
    uniZ=list(set(zt))#Convert to set to get repeated Z then back to list to get iterable.
    loc=f7(loc)
    uniZ=sorted(uniZ)
    senCou=0
    for l,z in zip(loc,uniZ):
        senCou=senCou+1
        xPoints = [z,z]
        yPoints = [min(xx),max(xx)]
        if l < 6:
            sub.plot(xPoints, yPoints, 'b',mew=6,label="Sensors")
        else:
            sub.plot(xPoints, yPoints, 'k',mew=6,label="Sensors")
    return senCou
def splitByEvent(info):
    """
    This is needed to split the rows of reader into each event. 
    @param info This is output of csv
    @return evts This is a vector containing the rows of the reader for each event
    """
    evtRows = []
    evts= []
    ePrev=info.next()["Event"]
    for row in info: 
        #print ePrev , "aaa " , row["Event"]
        if ePrev != row["Event"]:
         #   print "Inside ", row["Z"]
            evts.append(evtRows)
            evtRows = []
            ePrev=row["Event"]
        #print "Outside ", row["Z"]

        evtRows.append(row)
    return evts
if __name__ == "__main__":
    try:
        opts, args = getopt.getopt(sys.argv[1:],"i:t:o:l:h:v:")#symbol to read in the ":" specifies if required to run
    except getopt.GetoptError:
        print "Argument fail"
        sys.exit(2)
    #print opts
    isTrackView=1
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
            eventLow=float(arg)
        elif opt in ("-h"):
            eventHigh=float(arg)
        elif opt in ("-v"):
           isTrackView=float(arg)

        if isTrackView < 1: 
            isTrackView=False
        else:
            isTrackView=True

    with open(inputfileHits, 'rb') as f:
        reader = csv.DictReader(f)
        evtHits = splitByEvent(reader)
    with open(inputfileTracks, 'rb') as f:
        reader = csv.DictReader(f)
        evtTracks = splitByEvent(reader)
        #print "len",len(evtTracks[0])
    for eh,et in zip(evtHits,evtTracks):
   #     print eh
        #Now make the rows in columns to loop over the z axis.
        ehV=getRowsHits(eh)
        etV=getRowsTracks(et)
        if ehV[0][0] != etV[0][0]:
            print "Events don't match"
            raise
        if eventLow < ehV[0][0] < eventHigh:
            sc,tc=runFigureMakingForInputs(ehV,etV,isTrackView)
            if etV[1][-1] != tc:
                print "The number of track passed is ", etV[1][-1] , " and the number of track constructed ",tc, " Fail!"
                raise
            print tc ," tracks found for event ", ehV[0][0], " with ", sc , "planes"  
        #else:
         #   print "Event ", eh[0][0], " not created"

