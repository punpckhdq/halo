xvs.1.1
add r5.xyz, -v0, c[-92]
dph oPos.x, v0, c[-96]
dp3 r6.x, r5, v1
dph oPos.y, v0, c[-95]
mul r6.xyz, r6.x, v1
mad r6.xyz, r6.xyz, c[-92].w, -r5
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r11.xyz, c[-80]
mul r10.xyz, c[-81].yzxw, r11.zxyw
mad r10.xyz, -r11.yzxw, c[-81].zxyw, r10
dp3 oT0.x, r6, c[-81]
dp3 oT0.y, r6, c[-80]
dp3 oT0.z, r6, r10
dph oT1.x, v4, c[-79]
dph oT1.y, v4, c[-78]
dph oT2.x, v4, c[-77]
dph oT2.y, v4, c[-76]
dph oT3.x, v4, c[-75]
dph oT3.y, v4, c[-74]
dph r8.x, v0, c[-87]
dph r8.y, v0, c[-86]
dph r8.z, v0, c[-88]
add r8.xy, v4.w, -r8
dp3 r10.x, v1, -c[-91]
max r8.xyz, r8, v4.z
mul r8.xy, r8, r8
min r8.xyz, r8, v4.w
add r8.x, r8.x, r8.y
mul r8.z, r8.z, c[-85].x
min r8.x, r8, v4.w
add r8.xy, v4.w, -r8
max r10.x, r10.x, -r10.x
mul r8.xy, r8, r8
add r8.y, r8.y, -r8.x
add r10.y, v4.w, -r10.x
mad r8.w, c[-85].y, r8.y, r8.x
mul r8.w, r8.w, c[-85].z
add r8.zw, -r8, v4.w
mul r8.w, r8.z, r8.w
mul r10.xy, r10, r8.w
mul oD0.w, r8.w, c[-82].z
mul oD1, r10.xxxy, c[-82].z
