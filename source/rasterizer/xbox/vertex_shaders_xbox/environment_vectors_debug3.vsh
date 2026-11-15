xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r2.xyz, v1
mov r3.xyz, v2
mov r4.xyz, v3
dp3 r5.x, v7, r4
dp3 r5.y, v7, r3
dp3 r5.z, v7, r2
dp3 r6.x, r2, c[-90]
mad oD0.xyz, r5, c[-91].w, c[-91].w
dp3 r6.y, r2, c[-89]
dp3 r6.z, r2, c[-91]
mul oT0.xy, v4, c[-84].xy
mad oD1.xyz, r6, c[-91].w, c[-91].w
dp3 oT1.x, r3, r4
dp3 oT1.y, r3, r3
dp3 oT1.z, r3, r2
