xvs.1.1
add r5.xyz, -v0, c[-92]
dph oPos.x, v0, c[-96]
dp3 r6.x, r5, v1
mov oT2.xyz, v1                             
dph oPos.y, v0, c[-95]
mul r6.xyz, r6.x, v1
mad r6.xyz, r6.xyz, c[-92].w, -r5
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mul oT0.xy, v4, c[-84].xy
dp3 oT1.x, -c[-91], v3
dp3 oT1.y, -c[-91], v2
dp3 oT1.z, -c[-91], v1
dph r8.x, v0, c[-87]
mov oT3.xyz, r6                             
dph r8.y, v0, c[-86]
dph r8.z, v0, c[-88]
add r8.xy, v4.w, -r8
max r8.xyz, r8, v4.z
mul r8.xy, r8, r8
min r8.xyz, r8, v4.w
add r8.x, r8.x, r8.y
mul r8.z, r8.z, c[-85].x
min r8.x, r8, v4.w
add r8.xy, v4.w, -r8
mul r8.xy, r8, r8
add r8.y, r8.y, -r8.x
mad r8.w, c[-85].y, r8.y, r8.x
mul r8.w, r8.w, c[-85].z
add r8.zw, -r8, v4.w
mul oD0.w, r8.z, r8.w
