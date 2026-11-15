xvs.1.1
add r2.xyz, c[-79], -v0
add r9.xyz, c[-76], -v0
dp3 r3.w, r2, r2
mov oT2.xy, v8                              
dp3 r5.z, v1, -c[-73]
dp3 r10.w, r9, r9
rsq r1.w, r3.w                              
mad r5.x, r3.w, -c[-79].w, v4.w
mul r4.xyz, r2, r1.w
dp3 r6.x, r4, v1
rsq r1.w, r10.w                             
dp3 r7.w, r4, c[-78]
mul r11.xyz, r9, r1.w
mul r8.w, r7.w, c[-78].w
add r6.z, r8.w, c[-77].w
mad r5.y, r10.w, -c[-76].w, v4.w
dp3 r6.y, r11, v1
dp3 r0.w, r11, c[-78]
dp3 r5.w, v1, -c[-71]
mul r2.w, r0.w, c[-75].w
add r6.w, r2.w, c[-74].w
max r3, r5, v4.z
max r4, r6, v4.z
min r5, r3, v4.w
min r6, r4, v4.w
mov r7.xyz, c[-69]
mul r8.xy, r5, r6
mul r9.x, r8.x, r6.z
mul r10.y, r8.y, r6.w
mad r11.xyz, r9.x, c[-77], r7
mad r0.xyz, r10.y, c[-74], r11
mad r2.xyz, r5.z, c[-72], r0
mad oD0.xyz, r5.w, c[-70], r2
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mul oT0.xy, v4, c[-84].xy
mul oT1.xy, v4, c[-84].zw
dph r3.x, v0, c[-87]
dph r3.y, v0, c[-86]
dph r4.z, v0, c[-88]
add r4.xy, v4.w, -r3
max r5.xyz, r4, v4.z
mul r5.xy, r5, r5
min r6.xyz, r5, v4.w
add r7.x, r6.x, r6.y
mul r0.z, r6.z, c[-85].x
min r6.x, r7, v4.w
add r8.xy, v4.w, -r6
mul r9.xy, r8, r8
add r10.y, r9.y, -r9.x
mad r11.w, c[-85].y, r10.y, r9.x
mul r0.w, r11.w, c[-85].z
add r2.zw, -r0, v4.w
mul oD0.w, r2.z, r2.w
