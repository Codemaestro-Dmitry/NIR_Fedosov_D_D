#ifndef DRAWGRAFWINDOW_H
#define DRAWGRAFWINDOW_H

#include <QDialog>
#include <QPointF>
#include <QVector>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class drawgrafwindow; }
QT_END_NAMESPACE

struct NodeData {
    int id;
    QPointF pos;
    QString label;
};

struct EdgeData {
    int from;
    int to;
    double weightA; // вес from -> to
    double weightB; // вес to -> from (если двусторонняя и отличаются)
    bool directed;
    bool bidirectional;
};

class QGraphicsScene;
class QGraphicsLineItem;

class DrawGrafWindow : public QDialog
{
    Q_OBJECT
public:
    explicit DrawGrafWindow(QWidget *parent = nullptr);
    ~DrawGrafWindow() override;

signals:
    void graphSaved(const QString &folder);

private slots:
    // слоты, привязанные к элементам UI
    void on_loadBgButton_clicked();
    void on_addNodeButton_toggled(bool checked);
    void on_addEdgeButton_toggled(bool checked);
    void on_deleteButton_toggled(bool checked);
    void on_saveButton_clicked();
    void on_closeButton_clicked();



public:
    // события сцены (forwarded from CustomScene)
    void onSceneMousePress(class QGraphicsSceneMouseEvent *event);
    void onSceneMouseMove(class QGraphicsSceneMouseEvent *event);
    void onSceneMouseRelease(class QGraphicsSceneMouseEvent *event);
    void applyLightTheme();
    void applyDarkTheme();

private:
    Ui::drawgrafwindow *ui;

    // сцена и временные объекты
    QGraphicsScene *m_scene;
    QGraphicsLineItem *m_tempLine; // пунктир при drag
    QMap<int, class NodeItem*> m_idToItem;

    QVector<NodeData> m_nodes;
    QVector<EdgeData> m_edges;
    QString m_bgPath;

    int m_nextNodeId;
    enum Mode { Mode_None, Mode_AddNode, Mode_AddEdge, Mode_Delete } m_mode;
    int m_edgeFromId;

public:
    // вспомогательные
    void addNodeAt(const QPointF &pos);
    int nodeIdAt(const QPointF &pos);
    void startEdgeDrag(int nodeId, const QPointF &pos);
    void updateEdgeDrag(const QPointF &pos);
    void finishEdgeDrag(const QPointF &pos);
    void createEdge(int fromId, int toId, double weightA, double weightB, bool directed, bool bidir);
    void redrawAll();
    bool saveJsonToFolder(const QString &folder);
    bool copyBackgroundToFolder(const QString &folder);
    void nodeItemMoved(int id, const QPointF &newPos);
};

#endif
