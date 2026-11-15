xvs.1.1
add r4.xyz, c[-81], -v0
mov r4.w, c[-91].w
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r10.x, v4, c[-83]
dph r10.y, v4, c[-82]
dp3 oT1.x, r4, c[-80]
mul oT0.xy, r10, c[-84].xy
dp3 oT1.y, r4, c[-79]
dp3 oT1.z, r4, c[-78]
mad oT2.xyz, r4, -c[-81].w, r4.w
dp3 oT3.x, r4, v3
dp3 oT3.y, r4, v2
dp3 oT3.z, r4, v1
