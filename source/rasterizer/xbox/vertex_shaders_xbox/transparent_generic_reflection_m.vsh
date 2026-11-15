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
dp3 r4.x, r3, r2
dph oPos.w, r10, c[-93]
mul r5.xyz, r4.x, r2
mad r6.xyz, r5.xyz, c[-92].w, -r3
mov r7.xyz, c[-80]
mul r8.xyz, c[-81].yzxw, r7.zxyw
mad r9.xyz, -r7.yzxw, c[-81].zxyw, r8
dp3 oT0.x, r6, c[-81]
dp3 oT0.y, r6, c[-80]
dp3 oT0.z, r6, r9
dph oT1.y, v4, c[-78]
dph oT2.x, v4, c[-77]
dph oT2.y, v4, c[-76]
dph oT3.x, v4, c[-75]
dph oT3.y, v4, c[-74]
dph r11.x, r10, c[-87]
dph r11.y, r10, c[-86]
dph r0.z, r10, c[-88]
add r0.xy, v4.w, -r11
dp3 r11.x, r2, -c[-91]
max r3.xyz, r0, v4.z
mul r3.xy, r3, r3
min r4.xyz, r3, v4.w
add r3.x, r4.x, r4.y
mul r8.z, r4.z, c[-85].x
min r4.x, r3, v4.w
add r5.xy, v4.w, -r4
max r0.x, r11.x, -r11.x
mul r6.xy, r5, r5
add r7.y, r6.y, -r6.x
add r0.y, v4.w, -r0.x
mad r7.w, c[-85].y, r7.y, r6.x
mul r8.w, r7.w, c[-85].z
add r9.zw, -r8, v4.w
mul r10.w, r9.z, r9.w
mul r2.xy, r0, r10.w
mul oD0.w, r10.w, c[-82].z
mul oD1, r2.xxxy, c[-82].z
