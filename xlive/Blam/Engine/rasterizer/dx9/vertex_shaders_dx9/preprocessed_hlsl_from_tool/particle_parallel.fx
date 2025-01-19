struct VS_OUTPUT
{
    float4 oD0 : COLOR0;
    float4 oD1 : COLOR1;
    float4 oT0 : TEXCOORD0;
    float4 oT1 : TEXCOORD1;
    float4 oPos : POSITION;
    float4 oT7 : TEXCOORD7;
};
//global vs 2.0 vertex shader header for asm conversions. (preceded by VS_INPUT/VS_OUTPUT struct)
//not used if #define HLSL is at the top of the .vsc file.

float4 c[96*2+16] : register(c0);  // grab all the xbox specific constant register space

void hsge(out float4 dest, float4 src0, float4 src1)
{
	dest.x = (src0.x >= src1.x) ? 1.0f : 0.0f;
	dest.y = (src0.y >= src1.y) ? 1.0f : 0.0f;
	dest.z = (src0.z >= src1.z) ? 1.0f : 0.0f;
	dest.w = (src0.w >= src1.w) ? 1.0f : 0.0f;
}

void hslt(out float4 dest, float4 src0, float4 src1)
{
	dest.x = (src0.x < src1.x) ? 1.0f : 0.0f;
	dest.y = (src0.y < src1.y) ? 1.0f : 0.0f;
	dest.z = (src0.z < src1.z) ? 1.0f : 0.0f;
	dest.w = (src0.w < src1.w) ? 1.0f : 0.0f;
}

void hsincos(out float2 dest, in float angle)
{
	sincos(angle, dest.y, dest.x);
}

float dp2(float4 a, float4 b)
{
	return a.x*b.x+a.y*b.y;
}

float dp3(float4 a, float4 b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

float dp4(float4 a, float4 b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
}

VS_OUTPUT main(//input registers
	float4 va_position: POSITION0,
	float4 va_node_indices: BLENDINDICES0,
	float4 va_node_weights: BLENDWEIGHT0,
	float4 va_texcoord: TEXCOORD0,
	float4 va_normal: NORMAL0,
	float4 va_binormal: BINORMAL0,
	float4 va_tangent: TANGENT0,
	float4 va_anisotropic_binormal: BINORMAL1,
	float4 va_incident_radiosity: COLOR1,
	float4 va_secondary_texcoord: TEXCOORD1,
	float4 va_isq_secondary_position: POSITION1,
	float4 va_isq_secondary_node_indices: BLENDINDICES1,
	float4 va_isq_secondary_node_weights: BLENDWEIGHT1,
	float4 va_isq_select: TEXCOORD2,
	float4 va_color: COLOR0,
	float4 va_tint_factor: COLOR2,
	float4 va_dsq_plane: TEXCOORD3,
	float4 va_billboard_offset: TEXCOORD4,
	float4 va_billboard_axis: TEXCOORD5,
	float4 va_pca_cluster_id: TEXCOORD6,
	float4 va_pca_weights: TEXCOORD7) 
{
	VS_OUTPUT output;

//temps    
    float4 r0 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r1 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r3 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r4 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r5 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r6 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r7 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r8 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r9 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r10 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 r11 = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 r12 = float4(1.0f, 1.0f, 1.0f, 1.0f);
	int4 a0;  //only use .x
	
//output registers
    float4 oT0 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT1 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT3 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT4 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT5 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT6 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oT7 = float4(1.0f, 1.0f, 1.0f, 1.0f);

	float oFog = 1.0f;
	float oPts = 1.0f;
    
    float4 oD0 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 oD1 = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
	float4 oPos = float4(1.0f, 1.0f, 1.0f, 1.0f);


	













#define V_POINT_CORNERS va_position 
#define V_CENTER va_normal
#define V_SCALE va_binormal.x		   
#define V_ROTATION va_binormal.y
#define R_TEMP2 r9
#define V_FRAME_FADE va_node_weights.x
#define V_CORNER_EXPANDER va_tangent

#define V_PARTICLE_ZERO va_position.z

#define V_PARTICLE_PRIMARY_TEXCOORD va_texcoord
#define V_PARTICLE_SECONDARY_TEXCOORD va_anisotropic_binormal
#define V_PARTICLE_COLOR_RGB va_secondary_texcoord.xyzz 
#define V_PARTICLE_COLOR_ALPHA va_secondary_texcoord.wwww

#define R_FADE r4


#define V_DIRECTION va_node_indices

#define R_BASIS_FORWARD r6
#define R_BASIS_LEFT r7
#define R_DIRECTION r8






	r10 = V_POINT_CORNERS ;
 {
    float4 temp = (float4)((r10.xyyy * V_CORNER_EXPANDER.zwww ) + V_CORNER_EXPANDER.xyyy );
    	r10.x=temp.x;
    	r10.y=temp.y;
}
{
    float4 temp = (float4)(r10.xyyy * V_SCALE );
    	r10.x=temp.x;
    	r10.y=temp.y;
}









	hsincos(R_TEMP2.xy, V_ROTATION);


{
    float4 temp = (float4)(-R_TEMP2.yyyy );
    	R_TEMP2.z=temp.z;
}
{
    float4 temp = (float4)(dot(r10.xyz, R_TEMP2.xzww.xyz));
    	r11.x=temp.x;
}
{
    float4 temp = (float4)(dot(r10.xyz, R_TEMP2.yxww.xyz));
    	r11.y=temp.y;
}
{
    float4 temp = (float4)(c[15].yyyy  );
    	r11.z=temp.z;
}


{
    float4 temp = (float4)(V_CENTER );
    	r0.x=temp.x;
    	r0.y=temp.y;
    	r0.z=temp.z;
}



	R_DIRECTION = V_DIRECTION ;
 

{
    float4 temp = (float4)(r0 + -c[7]  );
    	r10.x=temp.x;
    	r10.y=temp.y;
    	r10.z=temp.z;
}

{
    float4 temp = (float4)(dot(R_DIRECTION.xyz, R_DIRECTION.xyz));
    	R_BASIS_FORWARD.w=temp.w;
}
{
    float4 temp = (float4)(rsqrt(R_BASIS_FORWARD.wwww ));
    	R_BASIS_FORWARD.w=temp.w;
}
{
    float4 temp = (float4)(R_DIRECTION.xyzz * R_BASIS_FORWARD.wwww );
    	R_BASIS_FORWARD.x=temp.x;
    	R_BASIS_FORWARD.y=temp.y;
    	R_BASIS_FORWARD.z=temp.z;
}

{
    float4 temp = (float4)(r10.yzxx * R_BASIS_FORWARD.zxyy );
    	R_BASIS_LEFT.x=temp.x;
    	R_BASIS_LEFT.y=temp.y;
    	R_BASIS_LEFT.z=temp.z;
}
{
    float4 temp = (float4)((-R_BASIS_FORWARD.yzxx * r10.zxyy ) + R_BASIS_LEFT );
    	R_BASIS_LEFT.x=temp.x;
    	R_BASIS_LEFT.y=temp.y;
    	R_BASIS_LEFT.z=temp.z;
}
{
    float4 temp = (float4)(dot(R_BASIS_LEFT.xyz, R_BASIS_LEFT.xyz));
    	R_BASIS_LEFT.w=temp.w;
}
{
    float4 temp = (float4)(rsqrt(R_BASIS_LEFT.wwww ));
    	R_BASIS_LEFT.w=temp.w;
}
{
    float4 temp = (float4)(R_BASIS_LEFT.xyzz * R_BASIS_LEFT.wwww );
    	R_BASIS_LEFT.x=temp.x;
    	R_BASIS_LEFT.y=temp.y;
    	R_BASIS_LEFT.z=temp.z;
}


{
    float4 temp = (float4)(r11.xxxx * R_BASIS_FORWARD );
    	R_TEMP2.x=temp.x;
    	R_TEMP2.y=temp.y;
    	R_TEMP2.z=temp.z;
}
{
    float4 temp = (float4)((r11.yyyy * R_BASIS_LEFT ) + R_TEMP2.xyzz );
    	R_TEMP2.x=temp.x;
    	R_TEMP2.y=temp.y;
    	R_TEMP2.z=temp.z;
}

{
    float4 temp = (float4)(dot(r10.xyz, r10.xyz));
    	r10.w=temp.w;
}
{
    float4 temp = (float4)(rsqrt(r10.wwww ));
    	r10.w=temp.w;
}
{
    float4 temp = (float4)(r10.xyzz * r10.wwww );
    	r10.x=temp.x;
    	r10.y=temp.y;
    	r10.z=temp.z;
}
{
    float4 temp = (float4)(dot(r10.xyz, R_BASIS_FORWARD.xyz));
    	r10.w=temp.w;
}
{
    float4 temp = (float4)(max(r10.wwww , -r10.wwww ));
    	r10.w=temp.w;
}
	R_FADE = c[4].wwww  + -r10.wwww ;
 





{
    float4 temp = (float4)(r0.xyzz + R_TEMP2.xyzz );
    	r0.x=temp.x;
    	r0.y=temp.y;
    	r0.z=temp.z;
}


{
	float temptemp;
	temptemp = dot(r0.xyz, c[0].xyz);
 	temptemp = temptemp + c[0].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	oPos.x=temp.x;
}
}
{
	float temptemp;
	temptemp = dot(r0.xyz, c[1].xyz);
 	temptemp = temptemp + c[1].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	oPos.y=temp.y;
}
}
{
	float temptemp;
	temptemp = dot(r0.xyz, c[2].xyz);
 	temptemp = temptemp + c[2].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	oPos.z=temp.z;
}
}
{
	float temptemp;
	temptemp = dot(r0.xyz, c[3].xyz);
 	temptemp = temptemp + c[3].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	oPos.w=temp.w;
}
}


{
	float temptemp;
	temptemp = dot(r0.xyz, c[13].xyz);
 	temptemp = temptemp + c[13].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	r8.x=temp.x;
}
}
{
	float temptemp;
	temptemp = dot(r0.xyz, c[14].xyz);
 	temptemp = temptemp + c[14].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	r8.y=temp.y;
}
}
{
	float temptemp;
	temptemp = dot(r0.xyz, c[12].xyz);
 	temptemp = temptemp + c[12].wwww ;
 {
    float4 temp = (float4)(temptemp );
    	r8.z=temp.z;
}
}
{
    float4 temp = (float4)(c[4].wwww  + -r8 );
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
}
{
    float4 temp = (float4)(max(r8 , c[15].yyyy  ));
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
}
{
    float4 temp = (float4)(r8 * r8 );
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
}
{
    float4 temp = (float4)(min(r8 , c[4].wwww  ));
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
}
{
    float4 temp = (float4)(r8.xxxx + r8.yyyy );
    	r8.x=temp.x;
}
{
    float4 temp = (float4)(min(r8 , c[4].wwww  ));
    	r8.x=temp.x;
}
{
    float4 temp = (float4)(c[4].wwww  + -r8 );
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
}
{
    float4 temp = (float4)(r8 * r8 );
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
}
{
    float4 temp = (float4)(r8.yyyy + -r8.xxxx );
    	r8.y=temp.y;
}
{
    float4 temp = (float4)((c[15].xxxx * r8.yyyy ) + r8.xxxx );
    	r8.w=temp.w;
}
{
    float4 temp = (float4)(r8 * c[15].zwzw  );
    	r8.z=temp.z;
    	r8.w=temp.w;
}
{
    float4 temp = (float4)(-r8 + c[4].wwww  );
    	r8.x=temp.x;
    	r8.y=temp.y;
    	r8.z=temp.z;
    	r8.w=temp.w;
}
{
    float4 temp = (float4)(r8.zzzz * r8.wwww );
    	r8.w=temp.w;
}

{
    float4 temp = (float4)(c[4].wwww  + -R_FADE.zzzz );
    	r10.x=temp.x;
}
{
    float4 temp = (float4)(r10.xxxx * r10.xxxx );
    	r10.x=temp.x;
}
{
    float4 temp = (float4)(r10.xxxx * r10.xxxx );
    	r10.x=temp.x;
}
{
    float4 temp = (float4)(c[4].wwww  + -r10.xxxx );
    	r10.x=temp.x;
}
{
    float4 temp = (float4)(-c[207].wwww );
    	r11.x=temp.x;
}
{
    float4 temp = (float4)(c[4].wwww  + r11.xxxx );
    	r10.y=temp.y;
}
{
    float4 temp = (float4)((c[207].wwww * r10.xxxx ) + r10.yyyy );
    	r10.x=temp.x;
}
{
    float4 temp = (float4)(r8.wwww * r10.xxxx );
    	r8.w=temp.w;
}






{
    float4 temp = (float4)(V_POINT_CORNERS.xyyy );
    	r10.x=temp.x;
    	r10.y=temp.y;
}
{
    float4 temp = (float4)((r10.xyyy * V_PARTICLE_PRIMARY_TEXCOORD.zwww ) + V_PARTICLE_PRIMARY_TEXCOORD.xyyy );
    	r11.x=temp.x;
    	r11.y=temp.y;
}
{
    float4 temp = (float4)((r11.xyyy * c[18 + 0].xyyy ) + c[18 + 0].zwww );
    	oT0.x=temp.x;
    	oT0.y=temp.y;
}
{
    float4 temp = (float4)((r10.xyyy * V_PARTICLE_SECONDARY_TEXCOORD.zwww ) + V_PARTICLE_SECONDARY_TEXCOORD.xyyy );
    	r11.x=temp.x;
    	r11.y=temp.y;
}
{
    float4 temp = (float4)((r11.xyyy * c[18 + 0].xyyy ) + c[18 + 0].zwww );
    	oT1.x=temp.x;
    	oT1.y=temp.y;
}

{
    float4 temp = (float4)(V_PARTICLE_COLOR_RGB );
    	oD1.x=temp.x;
    	oD1.y=temp.y;
    	oD1.z=temp.z;
}
{
    float4 temp = (float4)(V_PARTICLE_COLOR_ALPHA * r8.wwww );
    	oD1.w=temp.w;
}
{
    float4 temp = (float4)(V_FRAME_FADE );
    	oD0.x=temp.x;
    	oD0.y=temp.y;
    	oD0.z=temp.z;
    	oD0.w=temp.w;
}


output.oD0=oD0;
output.oD1=oD1;
output.oT0=oT0;
output.oT1=oT1;
output.oPos=oPos;
// cram anything additional here that needs to happen at the end of all vertex shaders.

	output.oT7.xyzw = oPos; //float4(oPos.z, oPos.w); 
	
	output.oPos=lerp(oPos, float4(oPos.x, oPos.y, c[10].z + c[10].w * oPos.w, oPos.w), 1.0f);

	return output;
}
