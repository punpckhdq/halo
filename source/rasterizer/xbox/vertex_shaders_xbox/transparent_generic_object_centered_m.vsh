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
dp3 r0.w, r11, r11
dph oPos.z, r10, c[-94]
dph oPos.y, r10, c[-95]
rsq r1.w, r0.w                              
dph oPos.w, r10, c[-93]
mul r2.xyz, r11, r1.w
mov r3.xyz, c[-80]
mul r4.xyz, c[-81].yzxw, r3.zxyw
mad r5.xyz, -r3.yzxw, c[-81].zxyw, r4
dp3 oT0.x, r2, c[-81]
dp3 oT0.y, r2, c[-80]
dp3 oT0.z, r2, r5
dph oT1.y, v4, c[-78]
dph oT2.x, v4, c[-77]
dph oT2.y, v4, c[-76]
dph oT3.x, v4, c[-75]
dph oT3.y, v4, c[-74]
dph r6.x, r10, c[-87]
dph r6.y, r10, c[-86]
dph r7.z, r10, c[-88]
add r7.xy, v4.w, -r6
max r8.xyz, r7, v4.z
mul r8.xy, r8, r8
min r9.xyz, r8, v4.w
add r10.x, r9.x, r9.y
mul r4.z, r9.z, c[-85].x
min r9.x, r10, v4.w
add r11.xy, v4.w, -r9
dp3 r7.x, r2, -c[-91]
mul r0.xy, r11, r11
add r3.y, r0.y, -r0.x
max r8.x, r7.x, -r7.x
mad r3.w, c[-85].y, r3.y, r0.x
add r8.y, v4.w, -r8.x
mul r4.w, r3.w, c[-85].z
add r5.zw, -r4, v4.w
mul r6.w, r5.z, r5.w
mul r9.xy, r8, r6.w
mul oD0.w, r6.w, c[-82].z
mul oD1, r9.xxxy, c[-82].z
