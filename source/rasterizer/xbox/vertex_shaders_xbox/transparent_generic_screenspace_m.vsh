xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2.x
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
add r6.x, v6.w, -v6.x
dph oT1.x, v4, c[-79]
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
dph r3.x, r10, c[-96]                       
dp3 r0.w, r11, r11
dph oPos.z, r10, c[-94]
dph oPos.y, r10, c[-95]
dph r3.y, r10, c[-95]                       
rsq r1.w, r0.w                              
dph oPos.w, r10, c[-93]
dph r3.zw, r10, c[-93]                      
dph oT1.y, v4, c[-78]
mul r2.xyz, r11, r1.w
mov oT0.w, r3                               
dp4 oT0.x, r3, c[-81]
dp4 oT0.y, r3, c[-80]
dph oT2.x, v4, c[-77]
dph oT2.y, v4, c[-76]
dph oT3.x, v4, c[-75]
dph oT3.y, v4, c[-74]
dph r4.x, r10, c[-87]
dph r4.y, r10, c[-86]
dph r5.z, r10, c[-88]
add r5.xy, v4.w, -r4
max r6.xyz, r5, v4.z
mul r6.xy, r6, r6
min r7.xyz, r6, v4.w
add r8.x, r7.x, r7.y
mul r3.z, r7.z, c[-85].x
min r7.x, r8, v4.w
add r9.xy, v4.w, -r7
dp3 r5.x, r2, -c[-91]
mul r10.xy, r9, r9
add r11.y, r10.y, -r10.x
max r6.x, r5.x, -r5.x
mad r0.w, c[-85].y, r11.y, r10.x
add r6.y, v4.w, -r6.x
mul r3.w, r0.w, c[-85].z
add r4.zw, -r3, v4.w
mul r5.w, r4.z, r4.w
mul r7.xy, r6, r5.w
mul oD0.w, r5.w, c[-82].z
mul oD1, r7.xxxy, c[-82].z
