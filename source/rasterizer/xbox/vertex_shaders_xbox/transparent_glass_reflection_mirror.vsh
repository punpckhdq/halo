xvs.1.1
dph oPos.x, v0, c[-96]
dph r10.x, v0, c[-96]                       
dph oPos.y, v0, c[-95]
dph r10.y, v0, c[-95]                       
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
dph r10.w, v0, c[-93]                       
mul oT0.xy, v4, c[-84].xy
dp3 oT1.x, -c[-91], v3
dp3 oT1.y, -c[-91], v2
dp3 oT1.z, -c[-91], v1
mov oT2.xyz, v1
add r10.xyw, r10.xyw, r10.w
dph r8.y, v0, c[-86]
dph r8.x, v0, c[-87]
mov oT3.w, r10                              
mul oT3.xy, r10, c[-84].zw
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
