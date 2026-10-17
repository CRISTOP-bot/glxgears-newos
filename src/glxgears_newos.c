/*
 * NEWOS framebuffer gears demo. The geometry is adapted from the public-
 * domain gears demo by Brian Paul; rendering is performed by TinyGL.
 *
 * This deliberately uses NEWOS's framebuffer ABI rather than GLX: no X11
 * server or GLX protocol exists in NEWOS yet.
 */
#include <math.h>
#include <TGL/gl.h>
#include "zbuffer.h"
#include "nshlib.h"

#define PI_F 3.14159265358979323846f
#define MAX_RENDER_W 640u
#define MAX_RENDER_H 480u

static const GLfloat red[4]   = {0.82f, 0.12f, 0.10f, 1.0f};
static const GLfloat green[4] = {0.15f, 0.78f, 0.20f, 1.0f};
static const GLfloat blue[4]  = {0.18f, 0.32f, 0.92f, 1.0f};
static const GLfloat white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
static const GLfloat light_pos[4] = {5.0f, 5.0f, 10.0f, 0.0f};

static GLfloat tooth_radius(GLfloat root, GLfloat tip, int i)
{
    switch (i & 3) {
    case 1: return root;
    case 2: return tip;
    case 3: return tip;
    default: return root;
    }
}

/* A lit, extruded gear with a real center opening and rectangular teeth. */
static void draw_gear(GLfloat inner, GLfloat root, GLfloat tip,
                      GLfloat width, int teeth)
{
    int i;
    const int steps = teeth * 4;
    const GLfloat half = width * 0.5f;
    const GLfloat step = (2.0f * PI_F) / (GLfloat)steps;

    /* Front and back annular faces. */
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (i = 0; i <= steps; ++i) {
        GLfloat a = (GLfloat)i * step;
        GLfloat r = tooth_radius(root, tip, i);
        glVertex3f(inner * cosf(a), inner * sinf(a), half);
        glVertex3f(r * cosf(a), r * sinf(a), half);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (i = 0; i <= steps; ++i) {
        GLfloat a = (GLfloat)i * step;
        GLfloat r = tooth_radius(root, tip, i);
        glVertex3f(r * cosf(a), r * sinf(a), -half);
        glVertex3f(inner * cosf(a), inner * sinf(a), -half);
    }
    glEnd();

    /* Outer wall follows the toothed outline. */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= steps; ++i) {
        GLfloat a = (GLfloat)i * step;
        GLfloat r = tooth_radius(root, tip, i);
        glNormal3f(cosf(a), sinf(a), 0.0f);
        glVertex3f(r * cosf(a), r * sinf(a), -half);
        glVertex3f(r * cosf(a), r * sinf(a), half);
    }
    glEnd();

    /* Smooth inner wall around the bore. */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= steps; ++i) {
        GLfloat a = (GLfloat)i * step;
        glNormal3f(-cosf(a), -sinf(a), 0.0f);
        glVertex3f(inner * cosf(a), inner * sinf(a), half);
        glVertex3f(inner * cosf(a), inner * sinf(a), -half);
    }
    glEnd();
}

static void set_material(const GLfloat color[4])
{
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
}

static void render_scene(GLfloat angle)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -24.0f);
    glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glTranslatef(-4.0f, -2.5f, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    set_material(blue);
    draw_gear(1.0f, 3.35f, 4.0f, 1.0f, 18);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(3.5f, -2.5f, 0.0f);
    glRotatef(-2.0f * angle - 10.0f, 0.0f, 0.0f, 1.0f);
    set_material(red);
    draw_gear(0.55f, 1.55f, 2.0f, 1.6f, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-3.8f, 4.0f, 0.0f);
    glRotatef(-2.0f * angle - 25.0f, 0.0f, 0.0f, 1.0f);
    set_material(green);
    draw_gear(0.65f, 1.55f, 2.0f, 0.8f, 10);
    glPopMatrix();
}

static unsigned long parse_frames(int argc, char **argv)
{
    int i;
    for (i = 1; i + 1 < argc; ++i) {
        if (nstrcmp(argv[i], "--frames") == 0) {
            unsigned long n = natou(argv[i + 1]);
            return n > 3600u ? 3600u : n;
        }
    }
    return 300u;
}

int main(int argc, char **argv)
{
    struct nsh_fbinfo info;
    struct nsh_fbwrite request;
    ZBuffer *zb;
    unsigned long width, height, frame, frame_limit;
    GLfloat angle = 0.0f;

    if (sys_fbinfo(&info) != 0 || !info.present || !info.width || !info.height) {
        nputs("glxgears-newos: no framebuffer; boot NEWOS with Limine\n");
        return 1;
    }

    width = info.width < MAX_RENDER_W ? info.width : MAX_RENDER_W;
    width &= ~3ul; /* TinyGL's Z buffer requires four-pixel alignment. */
    height = info.height < MAX_RENDER_H ? info.height : MAX_RENDER_H;
    if (width < 160u || height < 120u) {
        nputs("glxgears-newos: framebuffer is too small\n");
        return 1;
    }

    zb = ZB_open((GLint)width, (GLint)height, ZB_MODE_RGBA, 0);
    if (!zb) {
        nputs("glxgears-newos: TinyGL framebuffer allocation failed\n");
        return 1;
    }

    glInit(zb);
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-8.0, 8.0, -8.0 * (GLdouble)height / (GLdouble)width,
              8.0 * (GLdouble)height / (GLdouble)width, 5.0, 60.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glShadeModel(GL_SMOOTH);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);
    glClearColor(0.025f, 0.035f, 0.065f, 1.0f);

    request.x = (info.width - width) / 2u;
    request.y = (info.height - height) / 2u;
    request.w = width;
    request.h = height;
    request.pixels = (u64)zb->pbuf;
    request.pixlen = width * height * sizeof(PIXEL);
    frame_limit = parse_frames(argc, argv);
    nputs("glxgears-newos: TinyGL software renderer; GLX is not used\n");

    for (frame = 0; frame_limit == 0u || frame < frame_limit; ++frame) {
        render_scene(angle);
        if (sys_fbwrite(&request) != 0) {
            nputs("glxgears-newos: framebuffer write failed\n");
            glClose();
            ZB_close(zb);
            return 1;
        }
        angle += 2.0f;
        if (angle >= 360.0f)
            angle -= 360.0f;
        sys_sleep(16u);
    }

    glClose();
    ZB_close(zb);
    return 0;
}
