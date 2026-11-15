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
add r6.xyz, c[-79], -r10
mul r4.xyz, r3.x, r0
mad r5.xyz, r4.xyz, c[-92].w, -r2
add r3.xyz, c[-76], -r10
dp3 r7.w, r6, r6
mov oT3.xyz, r5                             
dp3 r7.z, r0, -c[-73]
dp3 r3.w, r3, r3
rsq r1.w, r7.w                              
mad r9.x, r7.w, -c[-79].w, v4.w
mul r8.xyz, r6, r1.w
dp3 r11.x, r8, r0
rsq r1.w, r3.w                              
dp3 r0.w, r8, -c[-78]
mul r4.xyz, r3, r1.w
mul r2.w, r0.w, c[-78].w
add r11.z, r2.w, c[-77].w
mad r9.y, r3.w, -c[-76].w, v4.w
dp3 r11.y, r4, r0
dp3 r5.w, r4, -c[-75]
mul r8.w, -r7.z, c[-82].z
mul r6.w, r5.w, c[-75].w
add r11.w, r6.w, c[-74].w
max r9.z, r7.z, r8.w
dp3 r9.w, r0, -c[-71]
max r4, r11, v4.z
max r3, r9, v4.z
min r11, r4, v4.w
mov r4.xyz, c[-69]
mul r6.xy, r3, r11
mul r7.xy, r6, r11.zw
mad r6.xyz, r7.x, c[-77], r4
mad r8.xyz, r7.y, c[-74], r6
mad r7.xyz, r3.z, c[-72], r8
mad oD0.xyz, r3.w, c[-70], r7
dph oPos.x, r10, c[-96]
dph oPos.y, r10, c[-95]
dph oPos.z, r10, c[-94]
dph oPos.w, r10, c[-93]
dph r8.x, v4, c[-83]
dph r8.y, v4, c[-82]
dp3 r9.w, r2, r2
mov oT0.xy, r8                              
mov oT2.xy, r8
mul oT1.xy, r8, c[-84]
rsq r1.w, r9.w                              
mul r10.xyz, r2, r1.w
dp3 r11, r10, r0
mul r0, r11, c[-81]
add oD1, r0, c[-80]
