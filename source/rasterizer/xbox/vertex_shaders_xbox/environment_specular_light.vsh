xvs.1.1
add r4.xyz, c[-81], -v0
mov r4.w, c[-91].w
add r5.xyz, c[-92], -v0
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r10.x, v4, c[-83]
dph r10.y, v4, c[-82]
mad oT1.xyz, -r4, c[-81].w, r4.w
mul oT0.xy, r10, c[-84].xy
dp3 oT2.x, r5, v3
dp3 oT2.y, r5, v2
dp3 oT2.z, r5, v1
dp3 oT3.x, r4, v3
dp3 oT3.y, r4, v2
dp3 oT3.z, r4, v1
