xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2.x
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
add r6.x, v6.w, -v6.x
dph oT0.x, v4, c[-81]
mad r7, r6.x, c[a0.x-36], r3
mad r8, r6.x, c[a0.x-35], r4
mad r9, r6.x, c[a0.x-34], r5
dph r10.x, v0, r7
dph r10.y, v0, r8
dph r10.z, v0, r9
dp3 r11.x, v1, r7
dp3 r11.y, v1, r8
dp3 r11.z, v1, r9
dph oPos.x, r10, c[-96]
dp3 r0.w, r11, r11
dph oPos.z, r10, c[-94]
dph oPos.y, r10, c[-95]
rsq r1.w, r0.w                              
dph oPos.w, r10, c[-93]
mul r2.xyz, r11, r1.w
dph oT0.y, v4, c[-80]
dph oT1.x, v4, c[-79]
dph oT1.y, v4, c[-78]
dph oT2.x, v4, c[-77]
dph oT2.y, v4, c[-76]
dph oT3.x, v4, c[-75]
dph oT3.y, v4, c[-74]
dph r3.x, r10, c[-87]
dph r3.y, r10, c[-86]
dph r4.z, r10, c[-88]
add r4.xy, v4.w, -r3
max r5.xyz, r4, v4.z
mul r5.xy, r5, r5
min r6.xyz, r5, v4.w
add r7.x, r6.x, r6.y
mul r0.z, r6.z, c[-85].x
min r6.x, r7, v4.w
add r8.xy, v4.w, -r6
dp3 r4.x, r2, -c[-91]
mul r9.xy, r8, r8
add r10.y, r9.y, -r9.x
max r5.x, r4.x, -r4.x
mad r11.w, c[-85].y, r10.y, r9.x
add r5.y, v4.w, -r5.x
mul r0.w, r11.w, c[-85].z
add r3.zw, -r0, v4.w
mul r4.w, r3.z, r3.w
mul r6.xy, r5, r4.w
mul oD0.w, r4.w, c[-82].z
mul oD1, r6.xxxy, c[-82].z
