!!ARBvp1.0

# standard model-view-projection transform
DP4 result.position.x, vertex.position, state.matrix.mvp.row[0];
DP4 result.position.y, vertex.position, state.matrix.mvp.row[1];
DP4 result.position.z, vertex.position, state.matrix.mvp.row[2];
DP4 result.position.w, vertex.position, state.matrix.mvp.row[3];

# plasma DX color swizzle and material color
MUL result.color, vertex.color.zyxw, program.local[0];

# pass the texture coordinate through
MOV result.texcoord, vertex.texcoord;

END
