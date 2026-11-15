xvs.1.1
mov r2.xyz, v7
dp3 r3.w, r2, r2
mov oT1.xy, v8                              
add r5.xyz, c[-92], -v0
dph oPos.x, v0, c[-96]
rsq r1.w, r3.w                              
dph oPos.y, v0, c[-95]
mul r4.xyz, r2, r1.w
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r6.x, v4, c[-83]
dph r6.y, v4, c[-82]
dp3 oT2.x, r5, v3
mul oT0.xy, r6, c[-84].xy
dp3 oT2.y, r5, v2
dp3 oT2.z, r5, v1
dp3 oT3.x, r4, v3
dp3 oT3.y, r4, v2
dp3 oT3.z, r4, v1
