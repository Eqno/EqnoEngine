#     �                GLSL.std.450                     main      4  n  �  �  �               �   
 GL_GOOGLE_cpp_style_line_directive    GL_GOOGLE_include_directive      main      
   DistributionGGX(f1;f1;       dnh   	   r        FresnelSchlick(f1;vf3;       dvh      f    	    GeometrySchlickGGX(f1;f1;        dnx      k        GeometrySmith(f1;f1;f1;      dnv      dnl      r    
 &   BRDF(vf3;vf3;vf3;vf3;f1;vf3;f1;      normal        light     !   view      "   color     #   roughness     $   fresnel   %   metallic      0   TBN(vf3;vf2;vf3;vf3;      ,   fragPosition      -   fragTexCoord      .   fragNormal    /   texNormal     2   a     4   _dnh      7   dnh_square    h   param     i   param     o   param     p   param     u   _half     x   dvh   {   dnh   ~   dnl   �   dnv   �   _fresnel      �   D     �   F     �   G     �   param     �   param     �   param     �   specular      �   kd    �   posDx     �   posDy     �   stx   �   sty   �   T     �   N     �   B     �   TBN     texBaseColor        baseColorSampler        fragTexCoord        texRoughness        roughnessSampler      "  texMetallic   #  metallicSampler   )  texNormal     *  normalSampler     0  DiffuseStrength   2  SpecularStength   4  outColor      <  i     F  LightData     F      id    F     type      F     intensity     F     pos   F     color     F     normal    F     viewMatrix    F     projMatrix    H  LightsData    H      num   H     object    J  lights    P  lightPos      W  lightColor    ]  lightNormal   c  lightIntensity    i  lightDir      n  fragPosition      r  viewDir   s  CameraData    s      pos   s     normal    u  camera    �  brdf      �  fragNormal    �  param     �  param     �  param     �  param     �  param     �  param     �  brdf      �  param     �  param     �  param     �  param     �  param     �  param     �  MaterialData      �      color     �     roughness     �     metallic      �  material      �  TransformData     �      modelMatrix   �     viewMatrix    �     projMatrix    �  transform     �  fragColor     �  fragTangent G    "       G    !      G          G    "       G    !      G  #  "       G  #  !      G  *  "       G  *  !      G  4         H  F      #       H  F     #      H  F     #      H  F     #      H  F     #       H  F     #   0   H  F        H  F     #   @   H  F           H  F        H  F     #   �   H  F           G  G     �   H  H      #       H  H     #      G  H     G  J  "       G  J  !      G  n         H  s      #       H  s     #      G  s     G  u  "       G  u  !       G  �        H  �      #       H  �     #      H  �     #      G  �     G  �  "       G  �  !      H  �         H  �      #       H  �            H  �        H  �     #   @   H  �           H  �        H  �     #   �   H  �           G  �     G  �  "       G  �  !      G  �        G  �             !                 !                         !                          !              !                 ! 
                               (           )            *      )   !  +      (   *   (   (   +     5       +     >   �I@+     C     �?+     G      @+     J   ��'7+     S     �@+     �     �@,     �   C   C   C   ,     �   5   5   5     �           +  �   �      +  �   �         �            �      �                        	                                             ;                  )   ;         ;          ;    #      +  �   '     ;    *      +     1     ?   3       ;  3  4       :            ;     :  +  :  =        E        
 F  :  �              E  E    G  F  '    H  �   G     I     H  ;  I  J        K     �     N  +  :  Q     +  :  S        T        +  :  Y        Z       +  :  _     +  :  e        f           m        ;  m  n       s           t     s  ;  t  u     ;  m  �     +     �  ���=,     �  �  �  �  +  �   �     +     �  ff�>,    �  �  �  �  C     �             �     �  ;  �  �       �  E  E  E     �     �  ;  �  �        �       ;  �  �     ;  m  �     6               �     ;         ;          ;     "     ;  (   )     ;     0     ;     2     ;  ;  <     ;  (   P     ;    W     ;  (   ]     ;     c     ;  (   i     ;  (   r     ;  (   �     ;  (   �     ;  *   �     ;  (   �     ;  (   �     ;     �     ;     �     ;  (   �     ;  (   �     ;  *   �     ;  (   �     ;  (   �     ;     �     ;     �     =        =  )       W          >      =        =  )       W           Q     !        >    !  =    $  #  =  )   %    W    &  $  %  Q     (  &     >  "  (  =    +  *  =  )   ,    W    -  +  ,  O     .  -  -                 /     E   .  >  )  /  >  0  1  >  2  S   =     5  0  =    6    �    7  6  5  P    8  >   >   >   >   �    9  7  8  >  4  9  >  <  =  �  >  �  >  �  @  A      �  B  �  B  =  :  C  <  |  �   D  C  A  K  L  J  =  =  �   M  L  �  N  O  D  M  �  O  ?  @  �  ?  =  :  R  <  A  T  U  J  Q  R  S  =     V  U  >  P  V  =  :  X  <  A  Z  [  J  Q  X  Y  =    \  [  >  W  \  =  :  ^  <  A  T  `  J  Q  ^  _  =     a  `       b     E   a  >  ]  b  =  :  d  <  A  f  g  J  Q  d  e  =     h  g  >  c  h  =  :  j  <  A  T  k  J  Q  j  S  =     l  k  =     o  n  �     p  l  o       q     E   p  >  i  q  A  T  v  u  =  =     w  v  =     x  n  �     y  w  x       z     E   y  >  r  z  =  :  {  <  A  K  |  J  Q  {  Q  =  �   }  |  �  N  ~  }  �   �  �      �  ~    �  �    =     �  n  >  �  �  =  )   �    >  �  �  =     �  �  >  �  �  =     �  )  >  �  �  9     �  0   �  �  �  �  =     �  ]  =     �  r  =    �    O     �  �  �            =     �    >  �  �  =     �  "  >  �  �  9     �  &   �  �  �  �  �  �  �  >  �  �  =     �  2  =     �  �  Q     �  �      Q     �  �     Q     �  �     P    �  �  �  �  C   �    �  �  �  =    �  W  �    �  �  �  =     �  c  �    �  �  �  =    �  4  �    �  �  �  >  4  �  �  �  �  �  =  :  �  <  A  K  �  J  Q  �  Q  =  �   �  �  �  N  �  �  �  �  �      �  �  �  �  �  �  =     �  n  >  �  �  =  )   �    >  �  �  =     �  �  >  �  �  =     �  )  >  �  �  9     �  0   �  �  �  �  =     �  i  =     �  r  =    �    O     �  �  �            =     �    >  �  �  =     �  "  >  �  �  9     �  &   �  �  �  �  �  �  �  >  �  �  =     �  2  =     �  �  Q     �  �      Q     �  �     Q     �  �     P    �  �  �  �  C   �    �  �  �  =    �  W  �    �  �  �  =     �  c  �    �  �  �  =    �  4  �    �  �  �  >  4  �  �  �  �  �  �  A  �  A  =  :  �  <  �  :  �  �  Q  >  <  �  �  >  �  @  =    �  4      �        �  �  >  4  �  �  8  6     
          7        7     	   �     ;     2      ;     4      ;     7      �     3   	   	   >  2   3        6      (   5      >  4   6   =     8   4   =     9   4   �     :   8   9   >  7   :   =     ;   2   =     <   2   �     =   ;   <   =     ?   7   =     @   2   =     A   2   �     B   @   A   �     D   B   C   �     E   ?   D   �     F   E   C        H         F   G   �     I   >   H        K      (   I   J   �     L   =   K   �  L   8  6               7        7        �     P     O   C   C   C   �     P   O      �     Q   C           R      +   Q   5   C        T         R   S   �     U   P   T   �     V      U   �  V   8  6               7        7        �     =     Y      =     Z      =     [      �     \   C   [   �     ]   Z   \   =     ^      �     _   ]   ^        `      (   _   J   �     a   Y   `   �  a   8  6               7        7        7        �     ;     h      ;     i      ;     o      ;     p      =     d           e      +   d   5   C   =     f      �     g   f   G   >  h   e   >  i   g   9     j      h   i   =     k           l      +   k   5   C   =     m      �     n   m   G   >  o   l   >  p   n   9     q      o   p   �     r   j   q   �  r   8  6     &          7        7         7     !   7     "   7     #   7     $   7     %   �  '   ;  (   u      ;     x      ;     {      ;     ~      ;     �      ;  (   �      ;     �      ;  (   �      ;     �      ;     �      ;     �      ;     �      ;  (   �      ;  (   �      �     v   !            w      E   v   >  u   w   =     y   u   �     z   !   y   >  x   z   =     |   u   �     }      |   >  {   }   �               >  ~      �     �      !   >  �   �   =     �   %   P     �   �   �   �        �      .   $   "   �   >  �   �   =     �   {   =     �   #   9     �   
   �   �   >  �   �   =     �   x        �      (   �   5   =     �   �   9     �      �   �   >  �   �   =     �   �   >  �   �   =     �   ~   >  �   �   =     �   #   >  �   �   9     �      �   �   �   >  �   �   =     �   �   =     �   �   �     �   �   �   =     �   �   �     �   �   �   =     �   �        �      +   �   5   C   �     �   �   �   =     �   ~        �      +   �   5   C   �     �   �   �        �      (   �   J   P     �   �   �   �   �     �   �   �   >  �   �   =     �   �   �     �   �   �   =     �   %   P     �   �   �   �        �      .   �   �   �   >  �   �   =     �   �   �     �   �   "   P     �   >   >   >   �     �   �   �   =     �   �   �     �   �   �   �  �   8  6     0       +   7  (   ,   7  *   -   7  (   .   7  (   /   �  1   ;  (   �      ;  (   �      ;  (   �      ;  (   �      ;  (   �      ;  (   �      ;  (   �      ;  �   �      =     �   ,   �     �   �   >  �   �   =     �   ,   �     �   �   >  �   �   =  )   �   -   Q     �   �       Q     �   �      P     �   �   �   5   �     �   �   >  �   �   =  )   �   -   Q     �   �       Q     �   �      P     �   �   �   5   �     �   �   >  �   �   A     �   �   �   =     �   �   =     �   �   �     �   �   �   A     �   �   �   =     �   �   =     �   �   �     �   �   �   �     �   �   �   A     �   �   �   =     �   �   A     �   �   �   =     �   �   �     �   �   �   A     �   �   �   =     �   �   A     �   �   �   =     �   �   �     �   �   �   �     �   �   �   P     �   �   �   �   �     �   �   �   >  �   �   =     �   .        �      E   �   >  �   �   =     �   �   =     �   �   =     �   �   =     �   �   �     �   �   �   �     �   �   �   �     �   �   �        �      E   �   >  �   �   =     �   �   =     �   �        �      D   �   �        �      E   �   >  �   �   =     �   �   =     �   �   =     �   �   Q     �   �       Q     �   �      Q     �   �      Q     �   �       Q     �   �      Q     �   �      Q     �   �       Q        �      Q       �      P       �   �   �   P       �   �   �   P       �        P  �           >  �     =  �     �   =       /   �         G   P     	  C   C   C   �     
    	            E   
  �                     E     �    8  