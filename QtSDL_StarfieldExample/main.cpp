#include <QApplication>
#include <QWidget>
#include <QShowEvent>
#include <QTimer>

#ifdef Q_WS_WIN
#include <SDL.h>
#elif defined Q_WS_X11
#include <SDL/SDL_main.h>
#include <SDL/SDL.h>
#undef main

#endif

/* PIEGE: main est redéfini par SDL.h comme SDL_Main.
          On retire donc la définition puisque Qt le gère
*/

#include <vector>
#include <cstdlib>
#include <ctime>

class SDLWidget : public QWidget
{
    Q_OBJECT

public:
    SDLWidget()
    :refreshTimer(0), windowInitialized(false), screen(0), StarNumbers(100)
    {
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);

        starfield.resize(100);
        initStarfield();

        // Le bon vieux mode 13h (simulé, c'est sûr <img src="../../bundles/tinymce/vendor/tiny_mce/plugins/emotions/img/langue.png" title=":p" alt=":p"> )
        resize(320, 200);

        refreshTimer = new QTimer(this);
        connect(refreshTimer, SIGNAL(timeout()), this, SLOT(onRefresh()));

        // simulons la vitesse de l'époque... <img src="../../bundles/tinymce/vendor/tiny_mce/plugins/emotions/img/clin.png" title=";)" alt=";)">
        refreshTimer->start(70);
    }

    virtual ~SDLWidget()
    {
        SDL_Quit();
    }

protected:
    virtual void showEvent(QShowEvent *e)
    {
        (void)e;

        if(!windowInitialized)
        {
            // C'est ici qu'on dis à SDL d'utiliser notre widget
            char windowid[64];
#ifdef Q_WS_WIN
            sprintf(windowid, "SDL_WINDOWID=0x%lx", reinterpret_cast<qlonglong>(winId()));
#elif defined Q_WS_X11
            sprintf(windowid, "SDL_WINDOWID=0x%lx", winId());
#else
            qFatal("Fatal: cast du winId() inconnu pour votre plate-forme; toute information est la bienvenue!");
#endif
            SDL_putenv(windowid);

            // Initialisation du système vidéo de SDL
            SDL_Init(SDL_INIT_VIDEO);
            screen = SDL_SetVideoMode(width(), height(), 32, SDL_SWSURFACE);
            windowInitialized = true;
        }
    }

private:
    struct Star
    {
        float x, y, z;
    };

private:
    Star generateStar()
    {
        Star s = {(::rand() / (static_cast<double>(RAND_MAX) + 1.0)) * 20-10,
                  (::rand() / (static_cast<double>(RAND_MAX) + 1.0)) * 20-10,
                  (::rand() / (static_cast<double>(RAND_MAX) + 1.0)) * 100};
        return s;
    }

    void initStarfield()
    {
        for(StarField::iterator it = starfield.begin();
            it != starfield.end();
            ++it)
        {
            *it = generateStar();
        }
    }

    void updateStarfield()
    {
        for(StarField::iterator it = starfield.begin();
            it != starfield.end();
            ++it)
        {
            --it->z;

            if(it->z <= 0)
            {
                *it = generateStar();
            }
        }
    }

    void drawStarfield()
    {
        int w = width();
        int hw = w >> 1;
        int h = height();
        int hh = h >> 1;

        for(StarField::iterator it = starfield.begin();
            it != starfield.end();
            ++it)
        {
            int screenX = static_cast<int>((it->x / it->z) * w + hw);
            int screenY = static_cast<int>((it->y / it->z) * h + hh);

            // ignore out of view stars
            if(screenX < 0 || screenX > 319 || screenY < 0 || screenY > 199)
                continue;

            putpixel(screen, screenX, screenY, SDL_MapRGBA(screen->format,
                0xfe, 0xef, 0xcc, 0xff));
        }
    }

    // ripped off SDL doc
    void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
    {
        int bpp = surface->format->BytesPerPixel;
        /* Here p is the address to the pixel we want to set */
        Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

        switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
        }
    }

private slots:
    void onRefresh()
    {
        if(windowInitialized && screen)
        {
            SDL_LockSurface(screen);
                // Nettoyage de l'écran
                SDL_FillRect(screen, NULL, 0);
                // Dessin du starfield
                drawStarfield();
            SDL_UnlockSurface(screen);

            // Rafraîchissement...
            SDL_UpdateRect(screen, 0, 0, 0, 0);

            // Et enfin, mise à jour des positions des étoiles
            updateStarfield();
        }
    }

private:
    QTimer *refreshTimer;
    bool windowInitialized;
    SDL_Surface *screen;

    typedef std::vector<Star> StarField;
    StarField starfield;
    const int StarNumbers;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ::srand(::time(NULL));

    SDLWidget *sdlw = new SDLWidget;
    sdlw->show();

    return app.exec();
}

#include "main.moc"
