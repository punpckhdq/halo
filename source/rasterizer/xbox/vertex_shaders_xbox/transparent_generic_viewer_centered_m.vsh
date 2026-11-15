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
add r3.xyz, -r10, c[-92]
dp3 r0.w, r11, r11
dph oPos.y, r10, c[-95]
dph oPos.x, r10, c[-96]
rsq r1.w, r0.w                              
dph oPos.z, r10, c[-94]
mul r2.xyz, r11, r1.w
dph oPos.w, r10, c[-93]
mov r4.xyz, c[-80]
mul r5.xyz, c[-81].yzxw, r4.zxyw
mad r6.xyz, -r4.yzxw, c[-81].zxyw, r5
dp3 oT0.x, r3, c[-81]
dp3 oT0.y, r3, c[-80]
dp3 oT0.z, r3, r6
dph oT1.y, v4, c[-78]
dph oT2.x, v4, c[-77]
dph oT2.y, v4, c[-76]
dph oT3.x, v4, c[-75]
dph oT3.y, v4, c[-74]
dph r7.x, r10, c[-87]
dph r7.y, r10, c[-86]
dph r8.z, r10, c[-88]
add r8.xy, v4.w, -r7
max r9.xyz, r8, v4.z
mul r9.xy, r9, r9
min r10.xyz, r9, v4.w
add r11.x, r10.x, r10.y
mul r5.z, r10.z, c[-85].x
min r10.x, r11, v4.w
add r0.xy, v4.w, -r10
dp3 r8.x, r2, -c[-91]
mul r3.xy, r0, r0
add r4.y, r3.y, -r3.x
max r9.x, r8.x, -r8.x
mad r4.w, c[-85].y, r4.y, r3.x
add r9.y, v4.w, -r9.x
mul r5.w, r4.w, c[-85].z
add r6.zw, -r5, v4.w
mul r7.w, r6.z, r6.w
mul r10.xy, r9, r7.w
mul oD0.w, r7.w, c[-82].z
mul oD1, r10.xxxy, c[-82].z
