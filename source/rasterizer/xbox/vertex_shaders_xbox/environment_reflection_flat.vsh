xvs.1.1
add r5.xyz, -v0, c[-92]
dph oPos.x, v0, c[-96]
dp3 r6.x, r5, v1
mov oT2.xyz, v1                             
dph oPos.y, v0, c[-95]
mul r6.xyz, r6.x, v1
mad r6.xyz, r6.xyz, c[-92].w, -r5
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r10.x, v4, c[-83]
mov oT3.xyz, r6                             
dph r10.y, v4, c[-82]
dp3 oT1.x, -c[-91], v3
mul oT0.xy, r10, c[-84].xy
dp3 oT1.y, -c[-91], v2
dp3 oT1.z, -c[-91], v1
