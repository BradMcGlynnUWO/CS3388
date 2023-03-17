# version 400

// Interpolated values from the vertex shaders
in vec 3 Normal ;
in vec 3 EyeDirection ;
in vec 3 LightDirection ;

out vec 4 color ;

 void main () {

 // material colors
 vec 4 diffuse = vec 4 ( 0.0, 1.0, 1.0, 1.0) ; // blue - green color
 vec 4 ambient = vec 4 ( 0.2, 0.2, 0.2, 1.0) ;
 vec 4 specular = vec 4 ( 0.7, 0.7, 0.7, 1.0 ) ;

 vec 3 n = normalize ( Normal ) ;
 vec 3 l = normalize ( LightDirection ) ;
 float cosTheta = clamp ( dot (n,l), 0,1 ) ;
 // ensure dot product is between 0 and 1

 vec 3 E = normalize ( EyeDirection ) ;
 vec 3 R = reflect ( -l , n ) ;
 float cosAlpha = clamp ( dot(E,R), 0,1 ) ;

 float alpha = 64 ;
 color = ambient + diffuse * cosTheta + specular * pow (cosAlpha, alpha) ;
 }