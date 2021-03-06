#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <D3d_matrix.h>
#include <parametric_equations.h>
#include <xwd_tools.h>
#include <unistd.h>
#include <string.h>

//General window vars
int width = 800;
int hieght = 800;

//3d vars
double half_angle = 30 * M_PI/180; 
double eye[3];
double center_of_interest[3];
double up[3];
double view[4][4];
double view_inv[4][4];


//Lighting vars
double light_source[3];
double ambient = 0.2;
double diffuse = 0.5;
double specularity = 50;
double spec_max; 

int fnum;


//Z-buffer stuff
double zbuffer[800][800];


//Sets all values in zbuffer to some number
int init_zbuffer(){
	int i, j;
	for (i = 0; i < width; i++){
		for (j = 0; j < hieght; j++){
		  zbuffer[i][j] = pow(100,100); 
		}
	}
	return 1;
}

//Initializes arrays and sets view matrix
int init(){
  //zbuffer
  init_zbuffer();
	
  //Eye
  eye[0] = 0;
  eye[1] = 0;
  eye[2] = -5;
	
  //Center of Interest
  center_of_interest[0] = 0;
  center_of_interest[1] = 0;
  center_of_interest[2] = 0;
	
  //Up is just the Eye[1] + 1
  up[0] = eye[0];
  up[1] = eye[1] + 1;
  up[2] = eye[2];
	
  //Light source
  light_source[0] = -20;
  light_source[1] = 5;
  light_source[2] = -20;
	
  //View Matrix
  D3d_make_identity(view);
  D3d_make_identity(view_inv);
  D3d_view(view,view_inv,eye,center_of_interest,up);
	
  //Spec max
  spec_max =  1 - ambient - diffuse;
  return 1;
}


int shade(double *rgb, double ambientplusdiffuse, double intensity){
  
  double new_rgb[3];

  double ratio;
  double t;
  if (intensity <= ambientplusdiffuse){
    ratio = intensity / ambientplusdiffuse;
    new_rgb[0] = ratio * rgb[0];
    new_rgb[1] = ratio * rgb[1];
    new_rgb[2] = ratio * rgb[2];
  }
  else{
    ratio = ((intensity - ambientplusdiffuse)/(1.0 - ambientplusdiffuse));
    t = 1.0 - ratio;
    new_rgb[0] = t * rgb[0] + ratio;
    new_rgb[1] = t * rgb[1] + ratio;
    new_rgb[2] = t * rgb[2] + ratio;
  }
  rgb[0] = new_rgb[0];
  rgb[1] = new_rgb[1];
  rgb[2] = new_rgb[2];

  return 1;
}
 


int light_model(double *vector1, double *vector2, double *xyz, double *rgb){
  //Vector from position to light source
  double lxyz [3] = {light_source[0] - xyz[0], light_source[1] - xyz[1], light_source[0] - xyz[2]};
  D3d_normalize(lxyz);

  //Normal vector from two input vectors
  double normal_vector[3];
  D3d_x_product(normal_vector, vector1, vector2);
  D3d_normalize(normal_vector);
  
  //Vector from postion
  double exyz [3] = {eye[0] - xyz[0], eye[1] - xyz[1], eye[2] - xyz[2]};
  D3d_normalize(exyz);

  //Reflection vector 
  double u = 2 * D3d_dot_product(normal_vector,lxyz);
  double r[3] = {
    ((u*normal_vector[0]) - lxyz[0]),
    ((u*normal_vector[1]) - lxyz[1]),
    ((u*normal_vector[2]) - lxyz[2])
  };
  D3d_normalize(r);
  

  double er = D3d_dot_product(exyz,r);
  double nl = D3d_dot_product(normal_vector,lxyz);
  double ne = D3d_dot_product(normal_vector,eye);

  if ((nl*ne) <= 0){
    er = 0;
    nl = 0;
  }
  if (er <= 0) {
    er = 0;
  }
  if (nl <= 0) {
    nl = 0;
  }
  
  double intensity = ambient + (diffuse * nl) + (spec_max * pow(er,specularity));
  shade (rgb, ambient + diffuse , intensity);

  return 1;
}


//FIX LIGHTING SHIT AND ADD TO THIS
int plot_3d (int map,int (*func)(double u1, double v1, double points[3]), double mat[4][4], double rgb[3]){
	//instance stuff
	int p[2];
	double xyz[3] = {0,0,0};
	double temp_rgb [3];
	double ulo, uhi, vlo, vhi;

	if (fnum == 8){
	  ulo = 0.0, uhi = 2*M_PI;
	  vlo = -(M_PI/2), vhi = M_PI/2;
	}
	else if (fnum == 9){
	  ulo = -1; uhi = 1;
	  vlo = -M_PI; vhi = M_PI;
	}

	double u, v;
	//Maps every point to matrix w/ translations
	for (u = ulo; u <= uhi; u+= 0.003){
	  for(v = vlo; v <= vhi ; v += 0.03) {
	    
	    //reinit temp_rgb
	    temp_rgb[0] = rgb[0];
	    temp_rgb[1] = rgb[1];
	    temp_rgb[2] = rgb[2];

	    //calls function, saves point into xyz, then makes necessary moves based on ident                  ity matrix and view matrix
	    func(u,v,xyz);
	    D3d_mat_mult_pt(xyz, mat, xyz);
	    D3d_mat_mult_pt(xyz, view, xyz);
	    
	    //Vectors for light_model
	    double t1[3], t2[3];

	    func(u + 0.1,v,t1);
	    D3d_mat_mult_pt(t1, mat, t1);
	    D3d_mat_mult_pt(t1, view, t1);
	    t1[0] = t1[0] - xyz[0];
	    t1[1] = t1[1] - xyz[1];
	    t1[2] = t1[2] - xyz[2];

	    func(u,v + 0.1,t2);
	    D3d_mat_mult_pt(t2, mat, t2);
	    D3d_mat_mult_pt(t2, view, t2);
	    t2[0] = t2[0] - xyz[0];
	    t2[1] = t2[1] - xyz[1];
	    t2[2] = t2[2] - xyz[2];

	    //change rgb w/ light
	    if (fnum == 8){
	      light_model(t2, t1, xyz, temp_rgb);
	    }
	    else if (fnum == 9){
	      light_model(t1, t2, xyz, temp_rgb);
	    }
	    
	    //translates xyz to xy plane
	    p[0] = (400/tan(half_angle)) * (xyz[0]/xyz[2]) + (width/2);
	    p[1] = (400/tan(half_angle)) * (xyz[1]/xyz[2]) + (hieght/2);
       
	    //saves zvalue of xyz into zbuffer[translatedx][translatedy]
	    if ((p[0] < 800 && p[0] > -1) && (p[1] < 800 && p[1] > -1)){
	      if (xyz[2] < zbuffer[p[0]] [p[1]]){
		zbuffer[p[0]] [p[1]] = xyz[2];
		set_xwd_map_color(map, p[0], p[1], temp_rgb[0], temp_rgb[1], temp_rgb[2]);
	      }
	    }
	  }
	}

	return 1;
}


int main(int argc, char *argv[]){

  //vars
  int filenum = 0;
  int Tn, Ttypelist[100];
  double Tvlist[100];
  double mat[4][4], mat_inv[4][4];
  int map;
  char file[25], filename[25];
  sprintf(file, "./output/%s", argv[1]);
  int framenum = atoi(argv[2]);

  //Initialize
  init();
  //Create map
  map = create_new_xwd_map(width, hieght);
		
  D3d_view(view, view_inv, eye, center_of_interest, up);

  //hyperbeloid
  double rgb[3] = {.5,0,.5};
  Tn = 0;

  
  int i;
  for (i = 0; i <= framenum; i++){
    Ttypelist[Tn] = SX;
    Tvlist[Tn] = 1.01; 
    Tn ++;
    Ttypelist[Tn] = SY;
    Tvlist[Tn] = 1.505;
    Tn ++;
  }
  D3d_make_movement_sequence_matrix (mat, mat_inv, Tn, Ttypelist, Tvlist);
  fnum = 9;
  plot_3d(map, f9, mat, rgb);

  sprintf(filename,"%s%s.xwd", file, argv[2]);
  xwd_map_to_named_xwd_file(map, filename);  
    
  //clear map
  map = create_new_xwd_map(width,hieght);
  exit(0);//Sucsess!
  
}


