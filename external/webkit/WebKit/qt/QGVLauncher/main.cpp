/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2009 Kenneth Christiansen <kenneth@webkit.org>
 * Copyright (C) 2009 Antonio Gomes <antonio.gomes@openbossa.org>
 * Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QDebug>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QNetworkRequest>
#include <QTextStream>
#include <QVector>
#include <QtGui>
#include <QtNetwork/QNetworkProxy>
#include <cstdio>
#include <qwebelement.h>
#include <qwebframe.h>
#include <qgraphicswebview.h>
#include <qwebpage.h>
#include <qwebsettings.h>
#include <qwebview.h>

static QUrl urlFromUserInput(const QString& string)
{
    QString input(string);
    QFileInfo fi(input);
    if (fi.exists() && fi.isRelative())
        input = fi.absoluteFilePath();

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
    return QUrl::fromUserInput(input);
#else
    return QUrl(input);
#endif
}

class WebView : public QGraphicsWebView {
    Q_OBJECT
    Q_PROPERTY(qreal yRotation READ yRotation WRITE setYRotation)

public:
    WebView(QGraphicsItem* parent = 0)
        : QGraphicsWebView(parent)
    {
        if (QApplication::instance()->arguments().contains("--cacheWebView"))
            setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
    void setYRotation(qreal angle)
    {
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        QRectF r = boundingRect();
        setTransform(QTransform()
            .translate(r.width() / 2, r.height() / 2)
            .rotate(angle, Qt::YAxis)
            .translate(-r.width() / 2, -r.height() / 2));
#endif
        m_yRotation = angle;
    }
    qreal yRotation() const
    {
        return m_yRotation;
    }

private:
    qreal m_yRotation;
};

class WebPage : public QWebPage {
    Q_OBJECT

public:
    WebPage(QObject* parent = 0)
        : QWebPage(parent)
    {
        applyProxy();
    }
    virtual QWebPage* createWindow(QWebPage::WebWindowType);

private:
    void applyProxy()
    {
        QUrl proxyUrl = urlFromUserInput(qgetenv("http_proxy"));

        if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
            int proxyPort = (proxyUrl.port() > 0) ? proxyUrl.port() : 8080;
            networkAccessManager()->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyPort));
        }
    }
};

class MainView : public QGraphicsView {
    Q_OBJECT

public:
    MainView(QWidget* parent)
        : QGraphicsView(parent)
        , m_mainWidget(0)
        , m_measureFps(QApplication::instance()->arguments().contains("--show-fps"))
        , m_numTotalPaints(0)
        , m_numPaintsSinceLastMeasure(0)
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        setFrameShape(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if (m_measureFps) {
            QTimer* fpsTimer = new QTimer(this);
            fpsTimer->setInterval(5000);
            m_totalStartTime = m_startTime = QTime::currentTime();
            connect(fpsTimer, SIGNAL(timeout()), this, SLOT(printFps()));
            fpsTimer->start();
        }
    }

    void setMainWidget(QGraphicsWidget* widget)
    {
        QRectF rect(QRect(QPoint(0, 0), size()));
        widget->setGeometry(rect);
        m_mainWidget = widget;
    }

    void resizeEvent(QResizeEvent* event)
    {
        QGraphicsView::resizeEvent(event);
        if (!m_mainWidget)
            return;
        QRectF rect(QPoint(0, 0), event->size());
        m_mainWidget->setGeometry(rect);
    }

    void paintEvent(QPaintEvent* event)
    {
        QGraphicsView::paintEvent(event);
        if (m_measureFps) {
            ++m_numPaintsSinceLastMeasure;
            ++m_numTotalPaints;            
        }
    }

    void setWaitCursor()
    {
        m_mainWidget->setCursor(Qt::WaitCursor);
    }

    void resetCursor()
    {
        m_mainWidget->unsetCursor();
    }

public slots:
    void flip()
    {
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        QSizeF center = m_mainWidget->boundingRect().size() / 2;
        QPointF centerPoint = QPointF(center.width(), center.height());
        m_mainWidget->setTransformOriginPoint(centerPoint);

        m_mainWidget->setRotation(m_mainWidget->rotation() ? 0 : 180);
#endif
    }

    void animatedFlip()
    {
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        QSizeF center = m_mainWidget->boundingRect().size() / 2;
        QPointF centerPoint = QPointF(center.width(), center.height());
        m_mainWidget->setTransformOriginPoint(centerPoint);

        QPropertyAnimation* animation = new QPropertyAnimation(m_mainWidget, "rotation", this);
        animation->setDuration(1000);

        int rotation = int(m_mainWidget->rotation());

        animation->setStartValue(rotation);
        animation->setEndValue(rotation + 180 - (rotation % 180));

        animation->start(QAbstractAnimation::DeleteWhenStopped);
#endif
    }

    void animatedYFlip()
    {
        emit flipRequest();
    }

    void printFps()
    {
        // note that this might have a bug if you measure right around midnight, but we can live with that
        QTime now = QTime::currentTime();
        int msecs = m_startTime.msecsTo(now);
        int totalMsecs = m_totalStartTime.msecsTo(now);
        int totalFps = totalMsecs ? m_numTotalPaints * 1000 / totalMsecs : 0;
        int curFps = msecs ? m_numPaintsSinceLastMeasure * 1000 / msecs : 0;
        qDebug("[FPS] From start: %d, from last paint: %d", totalFps, curFps);
        m_startTime = now;
        m_numPaintsSinceLastMeasure = 0;
    }

signals:
    void flipRequest();

private:
    QGraphicsWidget* m_mainWidget;
    bool m_measureFps;
    int m_numTotalPaints;
    int m_numPaintsSinceLastMeasure;
    QTime m_startTime;
    QTime m_totalStartTime;
};

class SharedScene : public QSharedData {
public:
    SharedScene()
    {
        m_scene = new QGraphicsScene;
        m_item = new WebView;
        m_item->setPage((m_page = new WebPage));

        m_scene->addItem(m_item);
        m_scene->setActiveWindow(m_item);
    }

    ~SharedScene()
    {
        delete m_item;
        delete m_scene;
        delete m_page;
    }

    QGraphicsScene* scene() const { return m_scene; }
    WebView* webView() const { return m_item; }

private:
    QGraphicsScene* m_scene;
    WebView* m_item;
    WebPage* m_page;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QExplicitlySharedDataPointer<SharedScene> other)
        : QMainWindow()
        , view(new MainView(this))
        , scene(other)
    {
        init();
    }

    MainWindow()
        : QMainWindow()
        , view(new MainView(this))
        , scene(new SharedScene())
    {
        init();
    }

    void init()
    {
        setAttribute(Qt::WA_DeleteOnClose);

        view->setScene(scene->scene());
        const QStringList arguments = QApplication::instance()->arguments();
        const int indexOfViewportUpdateMode = arguments.indexOf("--updateMode");
        if (indexOfViewportUpdateMode > 1 && indexOfViewportUpdateMode < arguments.count() - 1) {
            const QString updateMode = arguments[indexOfViewportUpdateMode+1] + "ViewportUpdate";
            view->setViewportUpdateMode(static_cast<QGraphicsView::ViewportUpdateMode>(QGraphicsView::staticMetaObject.enumerator(QGraphicsView::staticMetaObject.indexOfEnumerator("ViewportUpdateMode")).keysToValue(updateMode.toAscii())));
        }

        setCentralWidget(view);

        view->setMainWidget(scene->webView());

        connect(scene->webView(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        connect(scene->webView(), SIGNAL(titleChanged(const QString&)), this, SLOT(setWindowTitle(const QString&)));
        connect(scene->webView()->page(), SIGNAL(windowCloseRequested()), this, SLOT(close()));

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
        QStateMachine *machine = new QStateMachine(this);
        QState *s0 = new QState(machine);
        s0->assignProperty(scene->webView(), "yRotation", 0);

        QState *s1 = new QState(machine);
        s1->assignProperty(scene->webView(), "yRotation", 90);

        QAbstractTransition *t1 = s0->addTransition(view, SIGNAL(flipRequest()), s1);
        QPropertyAnimation *yRotationAnim = new QPropertyAnimation(scene->webView(), "yRotation", this);
        yRotationAnim->setDuration(1000);
        t1->addAnimation(yRotationAnim);

        QState *s2 = new QState(machine);
        s2->assignProperty(scene->webView(), "yRotation", -90);
        s1->addTransition(s1, SIGNAL(propertiesAssigned()), s2);

        QAbstractTransition *t2 = s2->addTransition(s0);
        t2->addAnimation(yRotationAnim);

        machine->setInitialState(s0);
        machine->start();
#endif

        resize(640, 480);
        buildUI();
    }

    void load(const QString& url)
    {
        QUrl deducedUrl = urlFromUserInput(url);
        if (!deducedUrl.isValid())
            deducedUrl = QUrl("http://" + url + "/");

        loadURL(deducedUrl);
    }

    QWebPage* page() const
    {
        return scene->webView()->page();
    }

protected slots:

    void openFile()
    {
        static const QString filter("HTML Files (*.htm *.html);;Text Files (*.txt);;Image Files (*.gif *.jpg *.png);;All Files (*)");

        QFileDialog fileDialog(this, tr("Open"), QString(), filter);
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::ExistingFile);
        fileDialog.setOptions(QFileDialog::ReadOnly);

        if (fileDialog.exec()) {
            QString selectedFile = fileDialog.selectedFiles()[0];
            if (!selectedFile.isEmpty())
                loadURL(QUrl::fromLocalFile(selectedFile));
        }
    }

    void changeLocation()
    {
        load(urlEdit->text());
    }

    void loadFinished(bool)
    {
        QUrl url = scene->webView()->url();
        urlEdit->setText(url.toString());

        QUrl::FormattingOptions opts;
        opts |= QUrl::RemoveScheme;
        opts |= QUrl::RemoveUserInfo;
        opts |= QUrl::StripTrailingSlash;
        QString s = url.toString(opts);
        s = s.mid(2);
        if (s.isEmpty())
            return;
        //FIXME: something missing here
    }

public slots:
    void newWindow(const QString &url = QString())
    {
        MainWindow* mw = new MainWindow();
        mw->load(url);
        mw->show();
    }

    void clone()
    {
        MainWindow* mw = new MainWindow(scene);
        mw->show();
    }

    void setWaitCursor()
    {
        view->setWaitCursor();
    }

    void resetCursor()
    {
        view->resetCursor();
    }

    void flip()
    {
        view->flip();
    }

    void animatedFlip()
    {
        view->animatedFlip();
    }

    void animatedYFlip()
    {
        view->animatedYFlip();
    }

private:

    void loadURL(const QUrl& url)
    {
        if (!url.isValid())
            return;
    
        urlEdit->setText(url.toString());
        scene->webView()->load(url);
        scene->webView()->setFocus(Qt::OtherFocusReason);
    }

    void buildUI()
    {
        QWebPage* page = scene->webView()->page();
        urlEdit = new QLineEdit(this);
        urlEdit->setSizePolicy(QSizePolicy::Expanding, urlEdit->sizePolicy().verticalPolicy());
        connect(urlEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

        QToolBar* bar = addToolBar("Navigation");
        bar->addAction(page->action(QWebPage::Back));
        bar->addAction(page->action(QWebPage::Forward));
        bar->addAction(page->action(QWebPage::Reload));
        bar->addAction(page->action(QWebPage::Stop));
        bar->addWidget(urlEdit);

        QMenu* fileMenu = menuBar()->addMenu("&File");
        fileMenu->addAction("New Window", this, SLOT(newWindow()), QKeySequence::New);
        fileMenu->addAction("Open File...", this, SLOT(openFile()), QKeySequence::Open);
        fileMenu->addAction("Clone Window", this, SLOT(clone()));
        fileMenu->addAction("Close Window", this, SLOT(close()), QKeySequence::Close);
        fileMenu->addSeparator();
        fileMenu->addAction("Quit", QApplication::instance(), SLOT(closeAllWindows()), QKeySequence(Qt::CTRL | Qt::Key_Q));

        QMenu* viewMenu = menuBar()->addMenu("&View");
        viewMenu->addAction(page->action(QWebPage::Stop));
        viewMenu->addAction(page->action(QWebPage::Reload));

        QMenu* testMenu = menuBar()->addMenu("&Tests");
        testMenu->addAction("Set Wait Cursor", this, SLOT(setWaitCursor()), QKeySequence("Ctrl+W"));
        testMenu->addAction("Reset Cursor", this, SLOT(resetCursor()), QKeySequence("Ctrl+Shift+W"));

        QMenu* fxMenu = menuBar()->addMenu("&Effects");
        fxMenu->addAction("Flip", this, SLOT(flip()));
        fxMenu->addAction("Animated Flip", this, SLOT(animatedFlip()), QKeySequence("Ctrl+R"));
        fxMenu->addAction("Animated Y-Flip", this, SLOT(animatedYFlip()), QKeySequence("Ctrl+Y"));
    }

private:
    MainView* view;
    QExplicitlySharedDataPointer<SharedScene> scene;

    QLineEdit* urlEdit;
};

QWebPage* WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow* mw = new MainWindow;
    mw->show();
    return mw->page();
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    if (app.arguments().contains("--help")) {
        qDebug() << "Usage: QGVLauncher [--url url] [--compositing] [--updateMode Full|Minimal|Smart|No|BoundingRect] [--cacheWebView]\n";
        return 0;
    }
    QString url = QString("file://%1/%2").arg(QDir::homePath()).arg(QLatin1String("index.html"));

    app.setApplicationName("GQVLauncher");

    QWebSettings::setObjectCacheCapacities((16 * 1024 * 1024) / 8, (16 * 1024 * 1024) / 8, 16 * 1024 * 1024);
    QWebSettings::setMaximumPagesInCache(4);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::enablePersistentStorage();

    const QStringList args = app.arguments();
    const int indexOfUrl = args.indexOf("--url");
    if (indexOfUrl > 0 && indexOfUrl < args.count() - 1)
        url = args.at(indexOfUrl+1);
    else if (args.count() > 1)
        url = args.at(1);
    if (args.contains("--compositing"))
        QWebSettings::globalSettings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);

    MainWindow* window = new MainWindow;
    window->load(url);

    for (int i = 2; i < args.count(); ++i)
        if (!args.at(i).startsWith("-") && !args.at(i - 1).startsWith("-"))
            window->newWindow(args.at(i));

    window->show();
    return app.exec();
}

#include "main.moc"
