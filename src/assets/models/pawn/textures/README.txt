Pawn texture atlas (UV channel "UVMap", one material per mesh)
Body 2048px, Sword 1024px.
  *_BaseColor  sRGB
  *_Normal     tangent space, OpenGL (+Y). Unreal: tick "Flip Green Channel".
  *_ORM        R=AmbientOcclusion G=Roughness B=Metallic (linear) - use this in Unreal / Unity URP mask setups
  *_Roughness / *_Metallic / *_AO  same data as separate greyscale maps
  T_Pawn_Body_Emissive  cyan eye; multiply by ~8 in the engine
