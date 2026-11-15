xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r2.xyz, v2
mov r3.xyz, v3
dp3 r10.x, r2, c[-90]
dp3 r10.y, r2, c[-89]
dp3 r10.z, r2, c[-91]
mad oD0.xyz, r10, c[-91].w, c[-91].w
dp3 r10.x, r3, c[-90]
dp3 r10.y, r3, c[-89]
dp3 r10.z, r3, c[-91]
mad oD1.xyz, r10, c[-91].w, c[-91].w
