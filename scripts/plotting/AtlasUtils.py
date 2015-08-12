# Port of AtlasUtils.C to python

from ROOT import *

def ATLAS_LABEL(x, y, color=1):
    l = TLatex()  #l.SetTextAlign(12); l.SetTextSize(tsize);
    l.SetNDC()
    l.SetTextFont(72)
    l.SetTextColor(color)
    l.DrawLatex(x,y,"ATLAS")


def myText(x, y, color, text):
  #tsize=0.05
    l = TLatex()  #l.SetTextAlign(12); l.SetTextSize(tsize);
    l.SetNDC()
    l.SetTextColor(color)
    l.DrawLatex(x,y,text)


