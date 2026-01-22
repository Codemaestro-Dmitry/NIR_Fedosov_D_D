#include"mainwindow.h"
#include"ui_mainwindow.h"
#include"drawgrafwindow.h"

#include<QFileDialog>
#include<QFile>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonValue>
#include<QProgressDialog>
#include<QMessageBox>
#include<QGraphicsEllipseItem>
#include<QGraphicsLineItem>
#include<QMouseEvent>
#include<QWheelEvent>
#include<QScrollBar>
#include<cmath>
#include<queue>
#include<limits>
#include<QFileDialog>
#include<QGraphicsPixmapItem>
#include<QString>
#include<windows.h>
#include<QTimer>
#include<QAction>
#include<QDesktopServices>
#include<QUrl>
#include<QImage>
#include<QPainter>
#include<QFileDialog>
#include<QSaveFile>
#include<QBuffer>

QString choise = "Дейкстра"; // Для выбора алгоритма
bool AppThema = 1; // Light - 1; Dark - 0;


// Пока что считается Эвклидово расстояние между точками, причем без учета высоты
// тут бы какой-нибудь API подключить, чтобы получать загруженность дорог, но это
// пока что не возможно (по крайней мере я не нашел где получить такие данные)
static double heuristicEuclid(const Vertex &a, const Vertex &b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow), m_drawWindow(nullptr)
{
    ui->setupUi(this);

    // Графическая сцена и view
    scene = new QGraphicsScene(this);
    ui->graphicsView_map->setScene(scene);
    ui->graphicsView_map->setRenderHint(QPainter::Antialiasing);

    // Установка фильтра событий для ловли кликов мышью по viewport (и зума/пан)
    ui->graphicsView_map->viewport()->installEventFilter(this);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // ############################### ВСЕ СВЯЗАННОЕ С MENU_BAR ##############################################

    QAction* importJSON = findChild<QAction*>("importJSON");
    QAction* podlozka = findChild<QAction*>("podlozka");
    QAction* dijstra_alg = findChild<QAction*>("dijstra_alg");
    QAction* A_star_alg = findChild<QAction*>("A_star_alg");
    QAction* ant_alg = findChild<QAction*>("ant_alg");
    QAction* Run = findChild<QAction*>("Run_2");
    QAction* drawgraf = findChild<QAction*>("drawgraf");
    QAction* aboutdialog = findChild<QAction*>("aboutdialog");
    QAction* helpdialog = findChild<QAction*>("helpdialog");
    QAction* savescene = findChild<QAction*>("savescene");


    // - - - - - - - - - - - - - - Настройка темы приложения - - - - - - - - - - - - - - - - - - -

    QAction* Light = findChild<QAction*>("Light");
    QAction* Dark = findChild<QAction*>("Dark");

    connect(Light, &QAction::triggered, this, &MainWindow::SetLightTema);
    connect(Dark, &QAction::triggered, this, &MainWindow::SetDarkTema);

    // И сразу установим светлую тему по умолчанию
    SetLightTema();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    connect(importJSON, &QAction::triggered, this, &MainWindow::on_importButton_clicked);
    importJSON->setShortcut(QKeySequence("Ctrl+I"));

    connect(podlozka, &QAction::triggered, this, &MainWindow::on_loadBgButton_clicked);
    podlozka->setShortcut(QKeySequence("Ctrl+B"));

    connect(dijstra_alg, &QAction::triggered, this, &MainWindow::ChoiseDijkstra);

    connect(A_star_alg, &QAction::triggered, this, &MainWindow::ChoiseA_star);

    connect(ant_alg, &QAction::triggered, this, &MainWindow::ChoiseAnt);

    connect(drawgraf, &QAction::triggered, this, &MainWindow::on_actionDrawGraph_triggered);

    connect(Run, &QAction::triggered, this, &MainWindow::on_buildButton_clicked);
    Run->setShortcut(QKeySequence("Ctrl+R"));

    connect(aboutdialog, &QAction::triggered, this, []() { QDesktopServices::openUrl(QUrl("https://github.com/Codemaestro-Dmitry/NIR_Fedosov_D_D"));});
    connect(helpdialog, &QAction::triggered, this, []() { QDesktopServices::openUrl(QUrl("https://github.com/Codemaestro-Dmitry/NIR_Fedosov_D_D/blob/main/Инструкция%20использования.md"));});
    connect(savescene, &QAction::triggered, this, &MainWindow::exportSceneAsImage);

    // #######################################################################################################
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


    statusBar()->showMessage("Готов");
}

MainWindow::~MainWindow()
{
    if (m_drawWindow) {
        m_drawWindow->deleteLater();
        m_drawWindow = nullptr;
    }
    delete ui;
}

void MainWindow::zoomAt(const QPointF &scenePos, double factor)
{
    // вычислить новый масштаб
    double newScale = currentScale * factor;
    if (newScale < minScale || newScale > maxScale) return;

    // сохранить позицию курсора в сцене до зума
    QPointF beforeCenter = scenePos;

    // выполнить масштабирование
    ui->graphicsView_map->scale(factor, factor);
    currentScale = newScale;

    // после масштабирования отцентрировать так, чтобы точка under cursor осталась под курсором
    // вычисляем смещение в сценовых координатах
    QPointF afterCenter = ui->graphicsView_map->mapToScene(ui->graphicsView_map->viewport()->rect().center());
    QPointF delta = afterCenter - beforeCenter;

    // сдвигаем скроллы чтобы компенсировать
    QScrollBar *h = ui->graphicsView_map->horizontalScrollBar();
    QScrollBar *v = ui->graphicsView_map->verticalScrollBar();
    h->setValue(h->value() + int(delta.x()));
    v->setValue(v->value() + int(delta.y()));
}

// Вот тут придумать как поменять, а то зум постоянно слетает и приходится заново приближать все
void MainWindow::resetViewToFit()
{
    if (!scene->items().isEmpty()) {
        ui->graphicsView_map->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        // обновим currentScale приблизительно (не точно)
        currentScale = 1.0;
    }
}

// (Взято из OpenSource проекта на Git). Почему-то с мышью норм работает, а с тачпадом нет
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // работаем с viewport у graphicsView_map
    if (watched == ui->graphicsView_map->viewport()) {
        // Колесо — масштаб
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *we = static_cast<QWheelEvent*>(event);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            QPoint numDegrees = we->angleDelta() / 8;
            int delta = numDegrees.y();
#else
            int delta = we->delta();
#endif
            QPoint viewPos = we->position().toPoint();
            QPointF scenePos = ui->graphicsView_map->mapToScene(viewPos);
            if (delta > 0) zoomAt(scenePos, zoomStep);
            else zoomAt(scenePos, 1.0 / zoomStep);
            return true; // событие обработано
        }

        // Нажатие кнопки мыши
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::MiddleButton) {
                // Начать панорамирование
                panning = true;
                lastPanPoint = me->pos();
                ui->graphicsView_map->viewport()->setCursor(Qt::ClosedHandCursor);
                return true;
            }
            if (me->button() == Qt::LeftButton) {
                // оставить обработку выбора вершин как раньше (по левому клику)
                QPoint viewPos = me->pos();
                QPointF scenePos = ui->graphicsView_map->mapToScene(viewPos);
                QList<QGraphicsItem*> itemsAt = scene->items(scenePos);
                for (QGraphicsItem *it : itemsAt) {
                    QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(it);
                    if (ellipse && ellipse->data(0).isValid()) {
                        int idx = ellipse->data(0).toInt();
                        if (startIndex == -1) {
                            startIndex = idx;
                            ellipse->setBrush(QBrush(Qt::blue));
                        } else if (goalIndex == -1 && idx != startIndex) {
                            goalIndex = idx;
                            ellipse->setBrush(QBrush(Qt::red));
                        } else if (idx == startIndex) {
                            ellipse->setBrush(QBrush(Qt::gray));
                            startIndex = -1;
                        } else if (idx == goalIndex) {
                            ellipse->setBrush(QBrush(Qt::gray));
                            goalIndex = -1;
                        } else {
                            clearSelectionColors();
                            startIndex = idx;
                            goalIndex = -1;
                            ellipse->setBrush(QBrush(Qt::blue));
                        }
                        return true;
                    }
                }
            }
        }

        // Перемещение мыши (пан)
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (panning) {
                QPoint delta = me->pos() - lastPanPoint;
                lastPanPoint = me->pos();
                QScrollBar *h = ui->graphicsView_map->horizontalScrollBar();
                QScrollBar *v = ui->graphicsView_map->verticalScrollBar();
                h->setValue(h->value() - delta.x());
                v->setValue(v->value() - delta.y());
                return true;
            }
        }

        // Отпуск кнопки мыши
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::MiddleButton && panning) {
                panning = false;
                ui->graphicsView_map->viewport()->setCursor(Qt::ArrowCursor);
                return true;
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// Слот загрузки графа с выбором файла
void MainWindow::on_importButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите JSON-файл с графом", QString(), "JSON Files (*.json);;All Files (*)");
    if (path.isEmpty())
    {
        statusBar()->showMessage("Файл не указан");
        return;
    }

    QProgressDialog progress("Проверка файла...", QString(), 0, 0, this);

    progress.setWindowModality(Qt::WindowModal);
    progress.setCancelButton(nullptr);
    progress.show();
    pause(5000); // Просто для создания ощущения, что программа "думает"
    qApp->processEvents();

    bool ok = parseJsonFile(path);

    progress.close();

    // [!] Проблема с сбросом подложки при загрузке нового графа. Мб сделать хранение пути к подложке в глобальной переменной и обновлять ее.

    if (ok)
    {
        QMessageBox::information(this, "Импорт", "Успешная загрузка");
        statusBar()->showMessage("Файл загружен");
        drawGraph();
        resetViewToFit(); // вписать граф после загрузки
    } else
    {
        QMessageBox::warning(this, "Импорт", "Некорректный файл");
        statusBar()->showMessage("Некорректный файл");
    }
}

// пауза без юлокировки интерфейса
void MainWindow::pause(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

// Слот запуск алгоритма
void MainWindow::on_buildButton_clicked()
{
    if (graph.isEmpty())
    {
        statusBar()->showMessage("Файл не указан");
        return;
    }
    if (startIndex == -1 || goalIndex == -1)
    {
        statusBar()->showMessage("Файл не указан");
        return;
    }

    QVector<int> path;
    if(choise == "A*")
    {
        path = a_star(startIndex, goalIndex);
        statusBar()->showMessage("ready!");
    }
    else if(choise == "Дейкстра")
    {
        path = dijkstra(startIndex, goalIndex);
        statusBar()->showMessage("READY!");
    }
    else
    {
        statusBar()->showMessage("Данный алгоритм временно не доступен");
        return;
    }

    if (path.isEmpty()) {
        statusBar()->showMessage("Нет возможного пути");
        return;
    }

    // Очистить предыдущее выделение и подсветить путь
    clearScene(); // Кстати проблема со сбросом подложки может быть тут    - - - - - - - - - - - - [!]
    drawGraph();
    highlightPath(path);
    statusBar()->setStyleSheet("background-color: rgb(11, 218, 81); color: black;");
    statusBar()->showMessage("Маршрут построен");
    pause(500);
    statusBar()->setStyleSheet("");
}

// Парсер JSONa (лучше не трогать, если не изменяется структура). Тут вообще планируется сделать что-то интересное
// по типу выбора нескольких вариантов форматов файлов. Было бы прикольно по фотке чтобы он определял вершины и ребра,
// но как искать тогда веса, вопрос, но можно как вариант оставить Евклидово расстояние. Ну а вообще, из более реалистичного,
// здесь можно подкрутить какой-нибудь OSM как в QGIS
bool MainWindow::parseJsonFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray data = f.readAll();
    f.close();

    QJsonParseError jerr;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jerr);
    if (jerr.error != QJsonParseError::NoError) return false;

    QJsonArray arr;
    QString coordType = QStringLiteral("scene");
    QString bgFileName;

    if (doc.isObject()) {
        QJsonObject root = doc.object();

        if (root.contains(QStringLiteral("coord_type")))
            coordType = root.value(QStringLiteral("coord_type")).toString();

        if (root.contains(QStringLiteral("bg_file")))
            bgFileName = root.value(QStringLiteral("bg_file")).toString();

        if (root.contains(QStringLiteral("nodes")) && root.value(QStringLiteral("nodes")).isArray())
            arr = root.value(QStringLiteral("nodes")).toArray();
        else {
            // также допускаем старый формат: сам массив в корне
            if (doc.isArray()) arr = doc.array();
            else return false;
        }
    } else if (doc.isArray()) {
        arr = doc.array();
    } else {
        return false;
    }

    bool isImagePixels = (coordType == QStringLiteral("image_pixels"));
    bool isRelativeBg  = (coordType == QStringLiteral("relative_bg"));
    bool isSceneCoords = (coordType == QStringLiteral("scene"));

    // Если формат требует фон (image_pixels/relative_bg), попробуем автоматически загрузить фон из той же папки, что JSON
    QString jsonFolder = QFileInfo(path).dir().absolutePath();
    if ((isImagePixels || isRelativeBg) && !bgFileName.isEmpty()) {
        QString candidate = QDir(jsonFolder).filePath(bgFileName);
        if (QFile::exists(candidate)) {
            // загрузим фон в mainwindow (перезаписываем bgPixmap/bgItem)
            QPixmap pm(candidate);
            if (!pm.isNull()) {
                bgPixmap = pm;
                // если уже есть bgItem — удалим, затем добавим новый
                if (bgItem) {
                    scene->removeItem(bgItem);
                    delete bgItem;
                    bgItem = nullptr;
                }
                bgItem = scene->addPixmap(pm);
                bgItem->setZValue(-10);
                bgItem->setData(0, QStringLiteral("bg"));
                // не меняем transform view — оставим расположение; sceneRect подстроим позже
            }
        }
    }

    QMap<int, Vertex> tmpGraph;

    // разбор вершин
    for (const QJsonValue &vv : arr) {
        if (!vv.isObject()) return false;
        QJsonObject obj = vv.toObject();

        if (!obj.contains(QStringLiteral("index"))) return false;
        Vertex ver;
        ver.index = obj.value(QStringLiteral("index")).toInt();

        double x_real = 0.0;
        double y_real = 0.0;

        // 1) image_pixels: координаты в пикселях оригинального изображения
        if (isImagePixels && obj.contains(QStringLiteral("img_x")) && obj.contains(QStringLiteral("img_y"))) {
            double img_x = obj.value(QStringLiteral("img_x")).toDouble();
            double img_y = obj.value(QStringLiteral("img_y")).toDouble();

            if (bgItem && !bgPixmap.isNull()) {
                QRectF br = bgItem->sceneBoundingRect();
                double imgW = bgPixmap.width();
                double imgH = bgPixmap.height();
                if (imgW > 0 && imgH > 0 && br.width() > 0 && br.height() > 0) {
                    double relx = img_x / imgW;
                    double rely = img_y / imgH;
                    x_real = br.left() + relx * br.width();
                    y_real = br.top()  + rely * br.height();
                } else {
                    // fallback на сценные координаты, если что-то не так
                    x_real = obj.contains(QStringLiteral("x_scene")) ? obj.value(QStringLiteral("x_scene")).toDouble() : 0.0;
                    y_real = obj.contains(QStringLiteral("y_scene")) ? obj.value(QStringLiteral("y_scene")).toDouble() : 0.0;
                }
            } else {
                // фон не загружен — используем сценные координаты, если есть
                x_real = obj.contains(QStringLiteral("x_scene")) ? obj.value(QStringLiteral("x_scene")).toDouble() : 0.0;
                y_real = obj.contains(QStringLiteral("y_scene")) ? obj.value(QStringLiteral("y_scene")).toDouble() : 0.0;
            }
        }
        // 2) relative_bg: координаты rel_x / rel_y (0..1)
        else if (isRelativeBg && obj.contains(QStringLiteral("rel_x")) && obj.contains(QStringLiteral("rel_y"))) {
            double relx = obj.value(QStringLiteral("rel_x")).toDouble();
            double rely = obj.value(QStringLiteral("rel_y")).toDouble();
            if (bgItem) {
                QRectF br = bgItem->sceneBoundingRect();
                x_real = br.left() + relx * br.width();
                y_real = br.top()  + rely * br.height();
            } else {
                // fallback
                x_real = obj.contains(QStringLiteral("x_scene")) ? obj.value(QStringLiteral("x_scene")).toDouble() : 0.0;
                y_real = obj.contains(QStringLiteral("y_scene")) ? obj.value(QStringLiteral("y_scene")).toDouble() : 0.0;
            }
        }
        // 3) прямые координаты x/y
        else if (obj.contains(QStringLiteral("x")) && obj.contains(QStringLiteral("y"))) {
            x_real = obj.value(QStringLiteral("x")).toDouble();
            y_real = obj.value(QStringLiteral("y")).toDouble();
        }
        // 4) сценные координаты
        else if (obj.contains(QStringLiteral("x_scene")) && obj.contains(QStringLiteral("y_scene"))) {
            x_real = obj.value(QStringLiteral("x_scene")).toDouble();
            y_real = obj.value(QStringLiteral("y_scene")).toDouble();
        } else {
            // ничего подходящего — отказ
            return false;
        }

        ver.x = x_real;
        ver.y = y_real;
        ver.z = obj.contains(QStringLiteral("z")) ? obj.value(QStringLiteral("z")).toDouble() : 0.0;

        // adj / weights
        if (!obj.contains(QStringLiteral("adj")) || !obj.contains(QStringLiteral("weights")))
            return false;
        QJsonArray adjArr = obj.value(QStringLiteral("adj")).toArray();
        QJsonArray wArr   = obj.value(QStringLiteral("weights")).toArray();
        if (adjArr.size() != wArr.size()) return false;
        for (int i = 0; i < adjArr.size(); ++i) {
            if (!adjArr[i].isDouble() || !wArr[i].isDouble()) return false;
            ver.adj.append(adjArr[i].toInt());
            ver.weights.append(wArr[i].toDouble());
        }

        tmpGraph.insert(ver.index, ver);
    }

    // проверка целостности (все соседи существуют)
    for (auto it = tmpGraph.constBegin(); it != tmpGraph.constEnd(); ++it) {
        for (int neigh : it.value().adj) {
            if (!tmpGraph.contains(neigh)) return false;
        }
    }

    // успешно распарсили — обновляем graph и флаги
    graph = tmpGraph;
    startIndex = -1;
    goalIndex = -1;

    // пометим, что координаты были привязаны к фону или сцене
    m_graphWasRelative = (!isSceneCoords);

    return true;
}


void MainWindow::drawGraph()
{
    // Очищаем сцену и вспомогательные карты
    clearScene(); // Еще один вариант, где очищается подложка - - - - - - - - - - - - - - [!]
    edgeItems.clear();
    nodeItems.clear();

    if (graph.isEmpty()) {
        scene->setSceneRect(0,0,100,100);
        return;
    }

    QMap<int, QPointF> sceneCoords; // index -> scene point

    if (m_graphWasRelative) {
        // v.x / v.y уже содержат реальные scene-координаты (вы их вычислили в parseJsonFile)
        for (auto it = graph.constBegin(); it != graph.constEnd(); ++it) {
            const Vertex &v = it.value();
            sceneCoords.insert(v.index, QPointF(v.x, v.y));
        }
    } else {
        // старое поведение: нормализация lon/lat -> scene (scale + padding)
        double minX = std::numeric_limits<double>::infinity();
        double minY = std::numeric_limits<double>::infinity();
        double maxX = -std::numeric_limits<double>::infinity();
        double maxY = -std::numeric_limits<double>::infinity();

        for (auto it = graph.constBegin(); it != graph.constEnd(); ++it) {
            const Vertex &v = it.value();
            if (v.x < minX) minX = v.x;
            if (v.x > maxX) maxX = v.x;
            if (v.y < minY) minY = v.y;
            if (v.y > maxY) maxY = v.y;
        }
        double width = maxX - minX; if (width < 1e-9) width = 1.0;
        double height = maxY - minY; if (height < 1e-9) height = 1.0;
        const double targetSize = 2000.0;
        double scale = targetSize / std::max(width, height);
        const double padding = 50.0;

        for (auto it = graph.constBegin(); it != graph.constEnd(); ++it) {
            const Vertex &v = it.value();
            double sx = (v.x - minX) * scale + padding;
            double sy = (v.y - minY) * scale + padding; // либо инвертируйте если у вас lon/lat с нужным направлением
            sceneCoords.insert(v.index, QPointF(sx, sy));
        }
    }


    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        const Vertex &v = it.value();
        for (int i = 0; i < v.adj.size(); ++i)
        {
            int to = v.adj[i];
            QPair<int,int> key = qMakePair(qMin(v.index, to), qMax(v.index, to));
            if (edgeItems.contains(key)) continue;
            if (!sceneCoords.contains(v.index) || !sceneCoords.contains(to)) continue;
            QPointF p1 = sceneCoords.value(v.index);
            QPointF p2 = sceneCoords.value(to);
            QGraphicsLineItem *line = scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), QPen(Qt::darkGray, 1));
            line->setZValue(0);
            edgeItems.insert(key, line);
        }
    }

    const double r = 7.0; // радиус вершины (в сценных единицах)
    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it) {
        const Vertex &v = it.value();
        QPointF p = sceneCoords.value(v.index);
        QGraphicsEllipseItem *ellipse = scene->addEllipse(p.x() - r, p.y() - r, r*2, r*2,
                                                              QPen(Qt::black), QBrush(Qt::gray));
        ellipse->setData(0, v.index);
        ellipse->setZValue(1);
        nodeItems.insert(v.index, ellipse);
    }

    if (bgItem && !bgPixmap.isNull()) {
        // вычислим bounding rect контента (исключая фон)
        QRectF contentRect;
        bool hasContent = false;

        // рёбра
        for (auto it = edgeItems.constBegin(); it != edgeItems.constEnd(); ++it) {
            QGraphicsLineItem *li = it.value();
            if (!li) continue;
            QRectF r = li->mapRectToScene(li->boundingRect());
            if (!hasContent) { contentRect = r; hasContent = true; }
            else contentRect |= r;
        }
        // вершины
        for (auto it = nodeItems.constBegin(); it != nodeItems.constEnd(); ++it) {
            QGraphicsEllipseItem *el = it.value();
            if (!el) continue;
            QRectF r = el->mapRectToScene(el->boundingRect());
            if (!hasContent) { contentRect = r; hasContent = true; }
            else contentRect |= r;
        }

        if (hasContent && contentRect.isValid()) {
            QSize targetSize = contentRect.size().toSize();
            if (targetSize.width() > 0 && targetSize.height() > 0) {
                // масштабируем оригинал bgPixmap, а не текущий pixmap (чтобы не терять качество при повторных масштабах)
                QPixmap scaled = bgPixmap.scaled(targetSize,
                                                 bgKeepAspect ? Qt::KeepAspectRatioByExpanding : Qt::IgnoreAspectRatio,
                                                 Qt::SmoothTransformation);
                bgItem->setPixmap(scaled);

                // позиционируем bgItem так, чтобы покрыть contentRect
                QPointF pos = contentRect.topLeft();
                if (bgKeepAspect) {
                    QSizeF s(scaled.width(), scaled.height());
                    QPointF offset((contentRect.width() - s.width()) / 2.0,
                                   (contentRect.height() - s.height()) / 2.0);
                    pos += offset;
                }
                bgItem->setPos(pos);
                bgItem->setZValue(-10);
            }
        }
    }

    scene->setSceneRect(scene->itemsBoundingRect());
    ui->graphicsView_map->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // сброс скейла внутренний, текущий масштаб логически 1
    currentScale = 1.0;
}

// Очистка сцены от графа
void MainWindow::clearScene()
{
    // Удаляем все элементы сцены кроме подложки (которая помечена data(0) == "bg")
    QList<QGraphicsItem*> items = scene->items();
    for (QGraphicsItem *it : items) {
        // если это pixmap и помечен как bg — пропускаем
        if (auto *pix = dynamic_cast<QGraphicsPixmapItem*>(it)) {
            if (pix->data(0).toString() == QStringLiteral("bg")) {
                continue;
            }
        }
        // иначе удаляем элемент
        scene->removeItem(it);
        delete it;
    }

    // очистим вспомогательные контейнеры
    edgeItems.clear();
    nodeItems.clear();

    // НЕ обнуляем bgItem/ bgPixmap — мы оставляем фон
}

// Очистка выделенного маршрута
void MainWindow::clearSelectionColors()
{
    if (startIndex != -1 && nodeItems.contains(startIndex))
    {
        nodeItems[startIndex]->setBrush(QBrush(Qt::gray));
    }
    if (goalIndex != -1 && nodeItems.contains(goalIndex))
    {
        nodeItems[goalIndex]->setBrush(QBrush(Qt::gray));
    }
    startIndex = -1;
    goalIndex = -1;
}

// Выделение найденного маршрута цветом
void MainWindow::highlightPath(const QVector<int> &path)
{
    for (int i = 0; i < path.size(); ++i)
    {
        int idx = path[i];
        if (nodeItems.contains(idx))
        {
            if (idx == startIndex) nodeItems[idx]->setBrush(QBrush(Qt::blue));
            else if (idx == goalIndex) nodeItems[idx]->setBrush(QBrush(Qt::red));
            else nodeItems[idx]->setBrush(QBrush(Qt::green));
        }
        if (i + 1 < path.size())
        {
            int a = path[i];
            int b = path[i+1];
            QPair<int,int> key = qMakePair(qMin(a,b), qMax(a,b));
            if (edgeItems.contains(key))
            {
                edgeItems[key]->setPen(QPen(Qt::red, 6));
            }
        }
    }
    if (startIndex != -1 && nodeItems.contains(startIndex)) nodeItems[startIndex]->setBrush(QBrush(Qt::blue));
    if (goalIndex != -1 && nodeItems.contains(goalIndex)) nodeItems[goalIndex]->setBrush(QBrush(Qt::red));
}

// - - - - - - - - - - - - - - - - Алгоритмы - - - - - - - - - - - - - - - -
QVector<int> MainWindow::a_star(int start, int goal)
{
    if (!graph.contains(start) || !graph.contains(goal)) return {};

    const double INF = std::numeric_limits<double>::infinity();
    QMap<int, double> gScore;
    QMap<int, double> fScore;
    QMap<int, int> cameFrom;

    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        gScore[it.key()] = INF;
        fScore[it.key()] = INF;
    }
    gScore[start] = 0.0;
    fScore[start] = heuristicEuclid(graph[start], graph[goal]);

    using Pair = std::pair<double,int>;
    struct Cmp
    {
        bool operator()(const Pair &a, const Pair &b) const
        {
            return a.first > b.first;
        }
    };
    std::priority_queue<Pair, std::vector<Pair>, Cmp> openSet;
    openSet.push({fScore[start], start});
    QSet<int> openSetContain;
    openSetContain.insert(start);

    while (!openSet.empty())
    {
        int current = openSet.top().second;
        openSet.pop();
        openSetContain.remove(current);

        if (current == goal)
        {
            QVector<int> path;
            int cur = goal;
            while (cameFrom.contains(cur))
            {
                path.prepend(cur);
                cur = cameFrom[cur];
            }
            path.prepend(start);
            return path;
        }

        const Vertex &cv = graph[current];
        for (int i = 0; i < cv.adj.size(); ++i)
        {
            int neighbor = cv.adj[i];
            double weight = cv.weights[i];
            double tentative_g = gScore[current] + weight;
            if (tentative_g < gScore[neighbor])
            {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_g;
                fScore[neighbor] = tentative_g + heuristicEuclid(graph[neighbor], graph[goal]);
                if (!openSetContain.contains(neighbor))
                {
                    openSet.push({fScore[neighbor], neighbor});
                    openSetContain.insert(neighbor);
                }
            }
        }
    }
    return {};
}

QVector<int> MainWindow::dijkstra(int start, int goal)
{
    if (!graph.contains(start) || !graph.contains(goal)) return {};

    const double INF = std::numeric_limits<double>::infinity();

    QMap<int, double> dist;
    QMap<int, int> prev;

    for (auto it = graph.constBegin(); it != graph.constEnd(); ++it)
    {
        dist[it.key()] = INF;
    }
    dist[start] = 0.0;

    using Pair = std::pair<double,int>;
    struct Cmp
    {
        bool operator()(const Pair &a, const Pair &b) const
        {
            return a.first > b.first;
        }
    };
    std::priority_queue<Pair, std::vector<Pair>, Cmp> pq;
    pq.push({0.0, start});
    QSet<int> visited;

    while (!pq.empty())
    {
        auto top = pq.top(); pq.pop();
        double d = top.first;
        int u = top.second;

        if (d > dist[u]) continue;
        if (visited.contains(u)) continue;
        visited.insert(u);

        if (u == goal) break;

        const Vertex &vu = graph[u];
        for (int i = 0; i < vu.adj.size(); ++i)
        {
            int v = vu.adj[i];
            double w = vu.weights[i];
            double nd = dist[u] + w;
            if (nd < dist[v])
            {
                dist[v] = nd;
                prev[v] = u;
                pq.push({nd, v});
            }
        }
    }

    if (!prev.contains(goal) && start != goal)
    {
        if (start == goal) return QVector<int>{start};
        return {};
    }

    QVector<int> path;
    int cur = goal;
    path.prepend(cur);
    while (cur != start)
    {
        if (!prev.contains(cur))
        {
            return {};
        }
        cur = prev[cur];
        path.prepend(cur);
    }
    return path;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//  - - - - - - - - - - - - - - - - Загрузка подложки - - - - - - - - - - - - - - - -
void MainWindow::on_loadBgButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Выберите изображение подложки", QString(),
                                                "Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)");
    if (path.isEmpty()) return;

    QPixmap pm(path);
    if (pm.isNull())
    {
        QMessageBox::warning(this, "Подложка", "Не удалось загрузить изображение");
        return;
    }

    // Сохраняем оригинал для последующего масштабирования
    bgPixmap = pm;

    // Если уже был bgItem — удалим старый (и оставим ссылку на новый)
    if (bgItem)
    {
        scene->removeItem(bgItem);
        delete bgItem;
        bgItem = nullptr;
    }

    // Добавим pixmap в сцену с отрицательным z, чтобы он был под графом
    bgItem = scene->addPixmap(pm);
    bgItem->setZValue(-10);
    bgItem->setOpacity(0.85); // можно регулировать прозрачность

    // Пометим фон — это нужно, чтобы мы могли его не трогать при clearScene()
    bgItem->setData(0, QStringLiteral("bg"));

    // Не масштабируем здесь насильно — масштаб подгонится при drawGraph() по содержимому,
    // либо, если нужно, можно тут вписать в viewport (см. вариант ниже).
    scene->setSceneRect(scene->itemsBoundingRect());
    ui->graphicsView_map->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);

    // Сохраняем путь
    bgPath = path; // если у тебя поле называется иначе — используй своё
}

// Функции, которые нужны для выбора алгоритма(если алгоритмов будет много, то надо думать что-то другое, а пока что и так сойдет)
void MainWindow::ChoiseDijkstra()
{
    choise = "Дейкстра";
    ui->dijstra_alg->setChecked(true);
    statusBar()->showMessage("Вы выбрали алгоритм Дейкстры");
    pause(1000);
    statusBar()->clearMessage();
    if(ui->A_star_alg->isChecked())
    {
        ui->A_star_alg->setChecked(false);
    }
}

void MainWindow::ChoiseA_star()
{
    choise = "A*";
    ui->A_star_alg->setChecked(true);
    if(ui->dijstra_alg->isChecked())
    {
        ui->dijstra_alg->setChecked(false);
    }
}

void MainWindow::ChoiseAnt()
{
    choise = "Муравьиный поиск";
}

void MainWindow::SetLightTema()
{
    setStyleSheet("QWidget { background-color: white; color: #333333; font-family: \"Segoe UI\", \"Roboto\", \"Arial\"; font-size: 12px; } QWidget#centralwidget, QFrame#centralwidget { background-color: #f8f9fa; border: 1px solid #e0e0e0; border-radius: 10px; } QGraphicsView, QGraphicsView#graphicsView_map { background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; } QMenuBar { background: transparent; spacing: 6px; padding: 6px 8px; } QMenuBar::item { background: #f0f0f0; padding: 6px 10px; margin: 0 3px; border-radius: 6px; } QMenuBar::item:selected { background: #e3f2fd; color: #1565c0; } QMenu { background-color: white; border: 1px solid #e0e0e0; padding: 6px; border-radius: 8px; color: #333333; } QMenu::item { padding: 6px 24px; border-radius: 6px; } QMenu::item:selected { background: #e3f2fd; color: #1565c0; } QPushButton { background: #f5f5f5; border: 1px solid #e0e0e0; color: #333333; padding: 6px 12px; border-radius: 8px; min-height: 28px; } QPushButton:hover { background: #e0e0e0; } QPushButton:pressed { background: #bdbdbd; border: 1px solid #9e9e9e; } QToolButton { background: transparent; border: none; padding: 4px; border-radius: 6px; } QToolButton:hover { background: #f0f0f0; } QStatusBar { background: #f5f5f5; border-top: 1px solid #e0e0e0; padding: 4px; color: #666666; } QLineEdit, QTextEdit, QPlainTextEdit { background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; padding: 6px; color: #333333; } QComboBox { background-color: white; border: 1px solid #e0e0e0; padding: 4px 8px; border-radius: 6px; } QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 20px; border-left: 1px solid #e0e0e0; } QProgressBar { background-color: #f0f0f0; border: 1px solid #e0e0e0; border-radius: 8px; text-align: center; padding: 2px; color: #333333; } QProgressBar::chunk { background: #2196f3; border-radius: 8px; } QListView, QTreeView { background-color: white; border: 1px solid #e0e0e0; border-radius: 6px; } QScrollBar:vertical { background: #f5f5f5; width: 10px; margin: 6px 2px 6px 2px; } QScrollBar::handle:vertical { background: #c0c0c0; min-height: 20px; border-radius: 5px; } QScrollBar::add-line, QScrollBar::sub-line { height: 0; } QToolTip { background-color: white; color: #333333; border: 1px solid #e0e0e0; padding: 6px; border-radius: 6px; } *:disabled { color: #9e9e9e; } QWidget.accent, QPushButton.accent { background: #2196f3; border: 1px solid #1976d2; color: white; }");
    ui->Light->setChecked(true);
    if(ui->Dark->isChecked())
    {
        ui->Dark->setChecked(false);
        if (m_drawWindow) m_drawWindow->applyLightTheme();
    }
    AppThema = 1;
}

void MainWindow::SetDarkTema()
{
    setStyleSheet("QWidget { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(18,22,30,200), stop:1 rgba(10,12,18,220)); color: rgba(240, 246, 255, 0.95); font-family: \"Segoe UI\", \"Roboto\", \"Arial\"; font-size: 12px; } QWidget#centralwidget, QFrame#centralwidget { background-color: rgba(255, 255, 255, 0.03); border: 1px solid rgba(255,255,255,0.06); border-radius: 10px; } QGraphicsView, QGraphicsView#graphicsView_map { background-color: rgba(255,255,255,0.02); border: 1px solid rgba(200,210,255,0.06); border-radius: 6px; } QMenuBar { background: transparent; spacing: 6px; padding: 6px 8px; } QMenuBar::item { background: rgba(255,255,255,0.02); padding: 6px 10px; margin: 0 3px; border-radius: 6px; } QMenuBar::item:selected { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgba(160,200,255,0.12), stop:1 rgba(200,160,255,0.10)); color: rgba(255,255,255,0.98); } QMenu { background-color: rgba(15,18,22,220); border: 1px solid rgba(255,255,255,0.05); padding: 6px; border-radius: 8px; color: rgba(240,246,255,0.95); } QMenu::item { padding: 6px 24px; border-radius: 6px; } QMenu::item:selected { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(140,180,255,0.14), stop:1 rgba(190,140,255,0.12)); color: rgba(10,10,12,0.98); } QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(130,180,255,0.12), stop:1 rgba(200,160,255,0.10)); border: 1px solid rgba(255,255,255,0.07); color: rgba(10,10,12,0.95); padding: 6px 12px; border-radius: 8px; min-height: 28px; } QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(130,180,255,0.20), stop:1 rgba(200,160,255,0.16)); } QPushButton:pressed { background-color: rgba(120,150,220,0.22); border: 1px solid rgba(100,120,180,0.25); } QToolButton { background: transparent; border: none; padding: 4px; border-radius: 6px; } QToolButton:hover { background: rgba(255,255,255,0.03); } QStatusBar { background: transparent; border-top: 1px solid rgba(255,255,255,0.03); padding: 4px; color: rgba(200,210,230,0.9); } QLineEdit, QTextEdit, QPlainTextEdit { background-color: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.05); border-radius: 6px; padding: 6px; color: rgba(240,246,255,0.95); } QComboBox { background-color: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.05); padding: 4px 8px; border-radius: 6px; } QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 20px; border-left: 1px solid rgba(255,255,255,0.03); } QProgressBar { background-color: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.04); border-radius: 8px; text-align: center; padding: 2px; color: rgba(240,246,255,0.95); } QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 rgba(120,170,255,0.6), stop:1 rgba(190,130,255,0.6)); border-radius: 8px; } QListView, QTreeView { background-color: rgba(255,255,255,0.015); border: 1px solid rgba(255,255,255,0.03); border-radius: 6px; } QScrollBar:vertical { background: transparent; width: 10px; margin: 6px 2px 6px 2px; } QScrollBar::handle:vertical { background: rgba(200,210,255,0.10); min-height: 20px; border-radius: 5px; border: 1px solid rgba(255,255,255,0.03); } QScrollBar::add-line, QScrollBar::sub-line { height: 0; } QToolTip { background-color: rgba(20,24,30,230); color: rgba(240,246,255,0.95); border: 1px solid rgba(255,255,255,0.05); padding: 6px; border-radius: 6px; } *:disabled { color: rgba(200,210,220,0.6); } QWidget.accent, QPushButton.accent { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(120,170,255,0.24), stop:1 rgba(200,140,255,0.20)); border: 1px solid rgba(180,190,255,0.10); color: rgba(8,8,10,0.95); }");
    ui->Dark->setChecked(true);
    if(ui->Light->isChecked())
    {
        ui->Light->setChecked(false);
        if (m_drawWindow) m_drawWindow->applyDarkTheme();
    }
    AppThema = 0;
}

void MainWindow::on_actionDrawGraph_triggered()
{
    // Если окно уже показано — просто поднимем его
        if (m_drawWindow) {
        m_drawWindow->raise();
        m_drawWindow->activateWindow();
        return;
    }

    // Создаём модельess окно с родителем — чтобы окно корректно относилось к приложению
    m_drawWindow = new DrawGrafWindow(this);

    // когда пользователь закроет окно, указатель должен сброситься:
    connect(m_drawWindow, &QObject::destroyed, this, [this]() {
        m_drawWindow = nullptr;
    });

    // автоматически удалять объект при закрытии окна — чтобы не утекало
    m_drawWindow->setAttribute(Qt::WA_DeleteOnClose);

    // показать окно (не блокирует главное)
    m_drawWindow->show();
    if(AppThema) m_drawWindow->applyLightTheme();
    else m_drawWindow->applyDarkTheme();
}


bool MainWindow::exportSceneAsImage()
{
    // Получаем прямоугольник, который нужно рендерить:
    // объединяем boundingRect элементов и фон (если он есть)
    QRectF contentRect = scene->itemsBoundingRect();

    // Найдём bgItem (если есть) и включим его в область экспорта
    for (QGraphicsItem *it : scene->items()) {
        if (auto *p = dynamic_cast<QGraphicsPixmapItem*>(it)) {
            if (p->data(0).toString() == QStringLiteral("bg")) {
                contentRect = contentRect.united(p->sceneBoundingRect());
                break;
            }
        }
    }

    // если пусто — попробуем сцена->sceneRect()
    if (!contentRect.isValid() || contentRect.isEmpty()) {
        contentRect = scene->sceneRect();
    }

    if (!contentRect.isValid() || contentRect.isEmpty()) {
        QMessageBox::warning(this, "Экспорт", "Нет содержимого для экспорта.");
        return false;
    }

    // Диалог выбора файла, если не передан путь
    QString filePath = "";
    int scale = 1;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", QString(),
                                                "PNG Image (*.png);;JPEG Image (*.jpg *.jpeg)");
        if (filePath.isEmpty()) return false;
    }

    // Определяем формат по расширению (по умолчанию PNG)
    QString ext = QFileInfo(filePath).suffix().toLower();
    QString format = (ext == "jpg" || ext == "jpeg") ? "JPG" : "PNG";

    // Размер итогового изображения в пикселях
    QSize imgSize(qMax(1, int(std::ceil(contentRect.width() * scale))),
                  qMax(1, int(std::ceil(contentRect.height() * scale))));

    // Если размер слишком большой — предупреждение (защита)
    const int MAX_PIXELS = 10000 * 10000; // предельное количество пикселей (пример)
    if (qint64(imgSize.width()) * qint64(imgSize.height()) > MAX_PIXELS) {
        if (QMessageBox::question(this, "Экспорт",
                                  "Размер изображения очень большой. Продолжить?") != QMessageBox::Yes) {
            return false;
        }
    }

    // Создаём QImage (ARGB32 для прозрачности)
    QImage image(imgSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Сопоставим область сцены contentRect в прямоугольник [0,0,imgSize]
    QRectF targetRect(0, 0, imgSize.width(), imgSize.height());
    scene->render(&painter, targetRect, contentRect);

    painter.end();

    // Сохраняем атомарно через QSaveFile
    QSaveFile sf(filePath);
    if (!sf.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Экспорт", "Не удалось открыть файл для записи: " + filePath);
        return false;
    }
    QByteArray imgData;
    QBuffer buf(&imgData);
    buf.open(QIODevice::WriteOnly);
    if (!image.save(&buf, format.toUtf8().constData())) {
        QMessageBox::warning(this, "Экспорт", "Ошибка конвертации изображения в формат " + format);
        return false;
    }
    if (sf.write(imgData) == -1) {
        QMessageBox::warning(this, "Экспорт", "Ошибка записи файла: " + filePath);
        return false;
    }
    if (!sf.commit()) {
        QMessageBox::warning(this, "Экспорт", "Не удалось сохранить файл: " + filePath);
        return false;
    }

    QMessageBox::information(this, "Экспорт", "Изображение успешно сохранено: " + filePath);
    return true;
}
