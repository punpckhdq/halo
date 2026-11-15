xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2.x
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
add r6.x, v6.w, -v6.x
mul oT0.xy, v4, c[-84].xy
mad r7, r6.x, c[a0.x-36], r3
mad r8, r6.x, c[a0.x-35], r4
mad r9, r6.x, c[a0.x-34], r5
dph r10.x, v0, r7
dph r10.y, v0, r8
dph r10.z, v0, r9
dp3 r11.x, v1, r7
dp3 r11.y, v1, r8
dp3 r11.z, v1, r9
add r3.xyz, c[-79], -r10
dp3 r0.w, r11, r11
mov r8.xyz, c[-69]
dp3 r4.w, r3, r3
rsq r1.w, r0.w                              
dph oPos.x, r10, c[-96]
mul r2.xyz, r11, r1.w
dp3 r6.z, r2, -c[-73]
rsq r1.w, r4.w                              
mad r6.x, r4.w, -c[-79].w, v4.w
mul r5.xyz, r3, r1.w
dp3 r7.x, r5, r2
dp3 r8.w, r5, c[-78]
add r11.xyz, c[-76], -r10
mul r9.w, r8.w, c[-78].w
add r7.z, r9.w, c[-77].w
dp3 r11.w, r11, r11
dph oPos.y, r10, c[-95]
dp3 r6.w, r2, -c[-71]
rsq r1.w, r11.w                             
mad r6.y, r11.w, -c[-76].w, v4.w
mul r0.xyz, r11, r1.w
dp3 r7.y, r0, r2
dp3 r2.w, r0, c[-78]
max r4, r6, v4.z
mul r3.w, r2.w, c[-75].w
add r7.w, r3.w, c[-74].w
min r6, r4, v4.w
max r5, r7, v4.z
min r7, r5, v4.w
mul r9.xy, r6, r7
mul r11.xy, r9, r7.zw
mad r0.xyz, r11.x, c[-77], r8
mad r2.xyz, r11.y, c[-74], r0
mad r3.xyz, r6.z, c[-72], r2
mad oD0.xyz, r6.w, c[-70], r3
dph oPos.z, r10, c[-94]
dph oPos.w, r10, c[-93]
mul oT1.xy, v4, c[-84].zw
dph r4.x, r10, c[-87]
dph r4.y, r10, c[-86]
dph r5.z, r10, c[-88]
add r5.xy, v4.w, -r4
max r6.xyz, r5, v4.z
mul r6.xy, r6, r6
min r7.xyz, r6, v4.w
add r8.x, r7.x, r7.y
mul r2.z, r7.z, c[-85].x
min r7.x, r8, v4.w
add r9.xy, v4.w, -r7
mul r10.xy, r9, r9
add r11.y, r10.y, -r10.x
mad r0.w, c[-85].y, r11.y, r10.x
mul r2.w, r0.w, c[-85].z
add r3.zw, -r2, v4.w
mul oD0.w, r3.z, r3.w
