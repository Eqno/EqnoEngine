#     �                 GLSL.std.450              
       main    9   �   �   �   �                �   
 GL_GOOGLE_cpp_style_line_directive    GL_GOOGLE_include_directive      main         ambientStrength   
   diffuseStrength      shininessStrength        specularStrength         ambient      diffuse      specular         i     %   LightData     %       id    %      type      %      intensity     %      pos   %      color     %      normal    %      viewMatrix    %      projMatrix    (   LightsData    (       num   (      object    *   lights    1   lightDir      9   fragPosition      =   viewDir   >   CameraData    >       pos   >      normal    @   camera    F   light     W   shadowMapPos      �   shadowMapZ    �   shadowMapSamplers     �   diff      �   fragNormal    �   halfwayDir    �   spec      �   texColor      �   baseColorSampler      �   fragTexCoord      �   outColor      �   fragColor     �   MaterialData      �       color     �      roughness     �      metallic      �   material      �   TransformData     �       modelMatrix   �      viewMatrix    �      projMatrix    �   transform   H  %       #       H  %      #      H  %      #      H  %      #      H  %      #       H  %      #   0   H  %         H  %      #   @   H  %            H  %         H  %      #   �   H  %            G  '      �   H  (       #       H  (      #      G  (      G  *   "       G  *   !      G  9          H  >       #       H  >      #      G  >      G  @   "       G  @   !       G  �   "       G  �   !      G  �         G  �   "       G  �   !      G  �         G  �          G  �         H  �       #       H  �      #      H  �      #      G  �      G  �   "       G  �   !      H  �          H  �       #       H  �             H  �         H  �      #   @   H  �            H  �         H  �      #   �   H  �            G  �      G  �   "       G  �   !           !                             +     	   ���>+        ��?+           @+        ��L?                       +            ,                                            +              !             #           $          
 %      !      #      #   $   $   +  !   &   �     '   %   &     (   !   '      )      (   ;  )   *         +      !     .      0      #   +     2      +     4         5      #      8      #   ;  8   9        >   #   #      ?      >   ;  ?   @      +     H         I         +     M         N         +     Y         Z      $   +     ^      +     c     �?+  !   i      +  !   o       +     r     ��+  !   |      +  !   �      +     �      ? 	 �                             �   �     �   �   &      �       �   ;  �   �          �            �       �   ;  8   �       	 �                              �   �      �       �   ;  �   �         �            �      �   ;  �   �         �         ;  �   �         �         ;  �   �        �               �      �   ;  �   �        �   $   $   $      �      �   ;  �   �      6               �     ;           ;     
      ;           ;           ;           ;           ;           ;           ;  0   1      ;  0   =      ;     F      ;     W      ;     �      ;     �      ;  0   �      ;     �      ;     �      >     	   >  
      >        >        >        >        >        >        �     �     �            �     �     =            |  !   "       A  +   ,   *      =  !   -   ,   �  .   /   "   -   �  /         �     =     3      A  5   6   *   2   3   4   =  #   7   6   =  #   :   9   �  #   ;   7   :     #   <      E   ;   >  1   <   A  5   A   @      =  #   B   A   =  #   C   9   �  #   D   B   C     #   E      E   D   >  =   E   =     G      A  I   J   *   2   G   H   =     K   J   =     L      A  N   O   *   2   L   M   =     P   O   �     Q   K   P   >  F   Q   =     R   F   =     S      �     T   R   S   =     U      �     V   U   T   >     V   =     X      A  Z   [   *   2   X   Y   =  $   \   [   =     ]      A  Z   _   *   2   ]   ^   =  $   `   _   �  $   a   \   `   =  #   b   9   Q     d   b       Q     e   b      Q     f   b      P     g   d   e   f   c   �     h   a   g   >  W   h   A     j   W   i   =     k   j   =     l   W   P     m   k   k   k   k   �     n   l   m   >  W   n   A     p   W   o   =     q   p   �  .   s   q   r   �  u       �  s   t   u   �  t   A     v   W   o   =     w   v   �  .   x   w   c   �  u   �  u   �  .   y   s      x   t   �  {       �  y   z   {   �  z   A     }   W   |   =     ~   }   �  .      ~   r   �  {   �  {   �  .   �   y   u      z   �  �       �  �   �   �   �  �   A     �   W   |   =     �   �   �  .   �   �   c   �  �   �  �   �  .   �   �   {   �   �   �  �       �  �   �   �   �  �   A     �   W   �   =     �   �   �  .   �   �   r   �  �   �  �   �  .   �   �   �   �   �   �  �       �  �   �   �   �  �   A     �   W   �   =     �   �   �  .   �   �   c   �  �   �  �   �  .   �   �   �   �   �   �  �       �  �   �   �   �  �   A     �   W   o   =     �   �   �     �   �   �   �     �   �   �   A     �   W   o   >  �   �   A     �   W   |   =     �   �   �     �   �   �   �     �   �   �   A     �   W   |   >  �   �   =     �      A  �   �   *   2   �      =     �   �   A  �   �   �   �   =  �   �   �   =     �   W   O  #   �   �   �             Q     �   �      Y     �   �   �   �   >  �   �   =     �   �   A     �   W   �   =     �   �   �  .   �   �   �   �  �       �  �   �   �   �  �   =  #   �   �     #   �      E   �   =  #   �   1   �     �   �   �        �      (   �      >  �   �   =     �   �   =     �   F   �     �   �   �   =     �   
   �     �   �   �   =     �      �     �   �   �   >     �   =  #   �   1   =  #   �   =   �  #   �   �   �     #   �      E   �   >  �   �   =  #   �   �     #   �      E   �   =  #   �   �   �     �   �   �        �      (   �      =     �           �         �   �   >  �   �   =     �   �   =     �   F   �     �   �   �   =     �      �     �   �   �   =     �      �     �   �   �   >     �   �  �   �  �   �  �   �  �   �     �     =     �      �     �   �   2   >     �   �     �     =  �   �   �   =  �   �   �   W     �   �   �   >  �   �   =     �      =     �      �     �   �   �   =     �      �     �   �   �   =     �   �   �     �   �   �   =     �   �   �     �   �   �   >  �   �   �  8  