int make_shader(int kind, char * source, int length);
int load_simple_shader(char * vertsrc, int vslen, char * fragsrc, int fslen);
int load_simple_shader2(char * geomsrc, int gslen, char * vertsrc, int vslen, char * fragsrc, int fslen);

void uniform_vec3(int loc, vec3 value);
void uniform_mat4(int loc, mat4 mat);
