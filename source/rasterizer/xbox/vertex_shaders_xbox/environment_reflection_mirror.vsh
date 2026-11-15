xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r10.w, v0, c[-93]                       
dph r10.x, v4, c[-83]
dph r10.y, v4, c[-82]
dp3 oT1.x, -c[-91], v3
mul oT0.xy, r10, c[-84].xy
dp3 oT1.y, -c[-91], v2
dp3 oT1.z, -c[-91], v1
mov oT2.xyz, v1
dph r10.x, v0, c[-96]
dph r10.y, v0, c[-95]
add r10.xyw, r10.xyw, r10.w
mul oT3.xy, r10, c[-84].zw
mov oT3.w, r10.w
