xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r2.xyz, v1
mov r3.xyz, v2
mov r4.xyz, v3
dp3 r5.x, v7, c[-90]
dp3 r5.y, v7, c[-89]
dp3 r5.z, v7, c[-91]
dp3 oD1.x, r2, r2
mad oD0.xyz, r5, c[-91].w, c[-91].w
dp3 oD1.y, r3, r3
dp3 oD1.z, r4, r4
