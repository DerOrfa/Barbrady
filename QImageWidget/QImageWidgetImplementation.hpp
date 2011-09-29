#ifndef QIMAGEWIDGETIMPLEMENTATION_HPP
#define QIMAGEWIDGETIMPLEMENTATION_HPP

#include <QImage>
#include <QWidget>
#include <QPainter>
#include <QtGui>
#include "QWidgetImplementationBase.hpp"
#include "QViewerCore.hpp"
#include "QMemoryHandler.hpp"
#include "Color.hpp"
#include "QtWidgetCommon.hpp"

namespace isis
{

namespace viewer
{

namespace qt
{

class QImageWidgetImplementation : public QWidget, public QWidgetImplementationBase
{
	Q_OBJECT
	struct ImageProperties {
		/**scaling, offset, size**/
		ViewPortType viewPort;
		Color colorHandler;
		util::PropertyMap propMap;
	};
	typedef std::map<boost::shared_ptr<ImageHolder>, ImageProperties> ImagePropertiesMapType;

public:
	QImageWidgetImplementation( QViewerCore *core, QWidget *parent = 0, QWidget *share = 0, PlaneOrientation orienation = axial );
	QImageWidgetImplementation( QViewerCore *core, QWidget *parent = 0, PlaneOrientation orientation = axial );

	virtual QWidgetImplementationBase *createSharedWidget( QWidget *parent, PlaneOrientation orienation = axial );

public Q_SLOTS:

	virtual void setZoom( float zoom );
	virtual void addImage( const boost::shared_ptr<ImageHolder> image );
	virtual bool removeImage( const boost::shared_ptr<ImageHolder> image );
	virtual void paintImage( boost::shared_ptr< ImageHolder > image );
	virtual void paintCrosshair() const;
	virtual void setScalingType( ScalingType scaling );

	virtual bool lookAtPhysicalCoords( const util::fvector4 &physicalCoords );
	virtual bool lookAtVoxelCoords( const util::ivector4 &voxelCoords );
	virtual void updateScene( bool center );
	virtual void setInterpolationType( InterpolationType interType ) { m_InterpolationType = interType; }

	virtual std::string getWidgetName() const;
	virtual void setWidgetName( const std::string &wName );

protected:
	void paintEvent( QPaintEvent *event );

	virtual void wheelEvent( QWheelEvent *e );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseReleaseEvent( QMouseEvent *e );
	virtual void mouseMoveEvent( QMouseEvent *e );

Q_SIGNALS:
	void redraw();
	void voxelCoordsChanged( util::ivector4 );
	void physicalCoordsChanged( util::fvector4 );
	void zoomChanged( float zoomFactor );


private:
	ImagePropertiesMapType m_ImageProperties;

	void emitMousePressEvent( QMouseEvent *e );
	bool isInViewPort( const ViewPortType &viewPort, QMouseEvent *e ) const;
	void recalculateTranslation();

	boost::shared_ptr<ImageHolder> getWidgetSpecCurrentImage() const;
	
	QMemoryHandler m_MemoryHandler;

	void commonInit();
	util::PropertyMap m_WidgetProperties;
	QPainter *m_Painter;
	InternalImageType m_InterpolationType;
	ScalingType m_ScalingType;
	ImageVectorType m_ImageVector;


};


}
}
} // end namespace
#endif
