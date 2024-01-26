def naive_conversion(x, base):
    t = ""
    if x==0:
        ab = 0
        cd = 0
        return cd,ab
    else:
        while x > 0:
            t = str(x % base) + t
            x //= base
            #t = [int(d) for d in str(y)]
        t=t.zfill(4) 
        ab = (t[2],t[3])
        abj = int("".join(ab))
        cd = (t[0],t[1])
    cdj = int("".join(cd))
    ret=cdj,abj
    return ret


g=naive_conversion(346,16)
print(g)