xvs.1.1
add r5.xyz, -v0, c[-92]
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r10.x, v4, c[-83]
mov oT1.w, r5.x                             
dph r10.y, v4, c[-82]
mov oT2.w, r5.y                             
mov oT1.x, v3.x
mul oT0.xy, r10, c[-84].xy
mov oT1.y, v2.x
mov oT1.z, v1.x
mov oT2.x, v3.y
mov oT2.y, v2.y
mov oT2.z, v1.y
mov oT3.x, v3.z
mov oT3.y, v2.z
mov oT3.z, v1.z
mov oT3.w, r5.z
