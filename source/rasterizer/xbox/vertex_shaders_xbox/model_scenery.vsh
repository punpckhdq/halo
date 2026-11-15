xvs.1.1
dph r2.x, v0, c[-36]
dph r2.y, v0, c[-35]
dph r2.z, v0, c[-34]
dp3 r3.x, v1, c[-36]
dp3 r3.y, v1, c[-35]
dp3 r3.z, v1, c[-34]
mov r8.xyz, c[-69]
mov oD0.w, v4.z                             
dp3 r4.x, r3, -c[-73]
dp3 r6.y, r3, -c[-71]
mul r5.w, -r4.x, c[-82].z
max r6.x, r4.x, r5.w
max r7.xy, r6, v4.z
mad r9.xyz, r7.x, c[-72], r8
mad oD0.xyz, r7.y, c[-70], r9
dph oPos.x, r2, c[-96]
dph oPos.y, r2, c[-95]
dph oPos.z, r2, c[-94]
dph oPos.w, r2, c[-93]
dph r10.x, v4, c[-83]
dph r10.y, v4, c[-82]
mov oT0.xy, r10
mul oT1.xy, r10, c[-84].xy
