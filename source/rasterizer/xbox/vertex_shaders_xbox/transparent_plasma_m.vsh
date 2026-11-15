xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2.x
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
add r6.x, v6.w, -v6.x
dph oT0.x, v0, c[-81]
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
dph oT0.z, v0, c[-79]
dph oT0.y, v0, c[-80]
rsq r1.w, r0.w                              
dph oT1.x, v0, c[-78]
mul r2.xyz, r11, r1.w
mad r4.xyz, r2, c[-81].z, r10
dph oT1.y, v0, c[-77]
dph oPos.x, r4, c[-96]
dph oPos.y, r4, c[-95]
dph oPos.z, r4, c[-94]
dph oPos.w, r4, c[-93]
dph oT1.z, v0, c[-76]
dp3 r5.w, r3, r3
dph r9.y, r4, c[-86]
dph r9.x, r4, c[-87]
rsq r1.w, r5.w                              
dph r10.z, r4, c[-88]
mul r6.xyz, r3, r1.w
dp3 r7, r6, r2
add r10.xy, v4.w, -r9
mul r8, r7, c[-82]
add oD1, r8, c[-83]
max r11.xyz, r10, v4.z
mul r11.xy, r11, r11
min r0.xyz, r11, v4.w
add r2.x, r0.x, r0.y
mul r7.z, r0.z, c[-85].x
min r0.x, r2, v4.w
add r3.xy, v4.w, -r0
mul r4.xy, r3, r3
add r5.y, r4.y, -r4.x
mad r6.w, c[-85].y, r5.y, r4.x
mul r7.w, r6.w, c[-85].z
add r8.zw, -r7, v4.w
mul oD0.w, r8.z, r8.w
