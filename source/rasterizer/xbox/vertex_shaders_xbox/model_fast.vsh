xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2
mov oD0.w, v4.z                             
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
add r6.x, v6.w, -v6.x
mad r7, r6.x, c[a0.x-36], r3
mad r8, r6.x, c[a0.x-35], r4
mad r9, r6.x, c[a0.x-34], r5
dph r10.x, v0, r7
dph r10.y, v0, r8
dph r10.z, v0, r9
dp3 r11.x, v1, r7
dp3 r11.y, v1, r8
dp3 r11.z, v1, r9
add r2.xyz, -r10, c[-92]
mul r0.xyz, r11, c[-84].w
dp3 r3.x, r2, r0
dp3 r6.x, r0, -c[-73]
mul r4.xyz, r3.x, r0
mad r5.xyz, r4.xyz, c[-92].w, -r2
dp3 r8.y, r0, -c[-71]
mul r7.w, -r6.x, c[-82].z
mov oT3.xyz, r5                             
max r8.x, r6.x, r7.w
max r9.xy, r8, v4.z
mov r11.xyz, c[-69]
mad r3.xyz, r9.x, c[-72], r11
mad oD0.xyz, r9.y, c[-70], r3
dph oPos.x, r10, c[-96]
dph oPos.y, r10, c[-95]
dph oPos.z, r10, c[-94]
dph oPos.w, r10, c[-93]
dph oFog.xw, r10, c[-88]
dph r3.x, v4, c[-83]
dph r3.y, v4, c[-82]
dp3 r2.w, r2, r2
mov oT0.xy, r3                              
mov oT2.xy, r3
mul oT1.xy, r3, c[-84]
rsq r1.w, r2.w                              
mul r3.xyz, r2, r1.w
dp3 r4, r3, r0
mul r5, r4, c[-81]
add oD1, r5, c[-80]
