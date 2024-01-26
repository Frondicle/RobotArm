#import math
import string
def naive_conversion(x, base):
    y = ""
    if x==0:
        ab= (0,0)
        abj = int(string.join(ab))
        cd = (0,0)
        cdj = int(string.join(cd))
    while x > 0:
        y = str(x % base) + y
        x //= base
        t = (int(d) for d in str(y))
        return t
    '''if len(t) == 1:
        ab = (0,t(0))
        abj = int(string.join(ab))
        cd = (0,0)
        cdj = int(string.join(cd))
    elif len(t) == 2:
        ab = (t(0),t(1))
        abj = int(string.join(ab))
        cd = (0,0)
        cdj = int(string.join(cd))
    elif len(t) == 3:
        ab = (t(1),t(2))
        abj = int(string.join(ab))
        cd = (0,t(0))
        cd = int(string.join(cd)) 
    elif len(t) == 4:
        ab = (t(2),t(3))
        ab = int(string.join(ab))
        cd = (t(0),t(1))
        cd = int(string.join(cd))
    ret=cd,ab
    return ret'''
N=naive_conversion(257,16)
print(N)
