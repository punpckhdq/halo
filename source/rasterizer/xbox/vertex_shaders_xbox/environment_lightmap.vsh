xvs.1.1
mov r2.xyz, v7
dp3 r3.x, r2, v3
dp3 r3.y, r2, v2
dp3 r3.z, r2, v1
dp3 r4.w, r2, r2
mov oT2.xy, v8                              
dph oPos.y, v0, c[-95]
dph oPos.x, v0, c[-96]
rsq r1.w, r4.w                              
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r5.x, v4, c[-83]
mov oD0.w, r4                               
dph r5.y, v4, c[-82]
mul oT3.xyz, r1.w, r3
mul oT0.xy, r5, c[-84].xy
mul oT1.xy, r5, c[-84].z
mov r6.xyz, c[-78]
dp3 r7.w, v1, c[-81]
dp3 r11.w, v1, c[-80]
mul r8.w, r7.w, c[-81].w
max r9.w, r8.w, v4.z
mad r10.xyz, r9.w, c[-79], r6
mul r0.w, r11.w, c[-80].w
max r2.w, r0.w, v4.z
mad oD0.xyz, r2.w, c[-79], r10
