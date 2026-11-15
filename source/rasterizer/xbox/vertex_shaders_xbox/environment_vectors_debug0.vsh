xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r2.xyz, v1
mov r3.xyz, v2
mov oT0.xyz, r2                             
mov r4.xyz, v3
mov oT1.xyz, r3                             
add r5.xyz, -v0, c[-92]
mov oT2.xyz, r4
mov oT3.xyz, r5
mad oD0.xyz, r2, c[-91].w, c[-91].w
mad oD1.xyz, r3, c[-91].w, c[-91].w
