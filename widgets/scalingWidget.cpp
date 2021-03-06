#include "scalingWidget.hpp"
#include "common.hpp"
#include "qviewercore.hpp"

namespace isis
{
namespace viewer
{
namespace widget
{

ScalingWidget::ScalingWidget( QWidget *parent, isis::viewer::QViewerCore *core )
	: QDialog( parent ),
	  m_ViewerCore( core )
{
	m_Interface.setupUi( this );
	setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::FramelessWindowHint );
	connect( m_Interface.min, SIGNAL( valueChanged( double ) ), this, SLOT( minChanged( double ) ) );
	connect( m_Interface.max, SIGNAL( valueChanged( double ) ), this, SLOT( maxChanged( double ) ) );
	connect( m_Interface.scaling, SIGNAL( valueChanged( double ) ), this, SLOT( scalingChanged( double ) ) );
	connect( m_Interface.offset, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged( double ) ) );
	connect( m_Interface.resetButton, SIGNAL( clicked() ), this, SLOT( reset() ) );
	connect( m_Interface.autoButton, SIGNAL( clicked() ), this, SLOT( autoScale() ) );
}

void ScalingWidget::synchronize()
{
	if( m_ViewerCore->getCurrentImage().get() ) {
		boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
		m_Interface.offset->setMaximum( std::numeric_limits<double>::max() );
		m_Interface.offset->setMinimum( std::numeric_limits<double>::min() );
		m_Interface.scaling->setMinimum( 0.0 );
		m_Interface.scaling->setMaximum( 100 );
		m_Interface.min->setMinimum( std::numeric_limits<double>::min() );
		m_Interface.min->setMaximum( std::numeric_limits<double>::max() );
		m_Interface.max->setMinimum( std::numeric_limits<double>::min() );
		m_Interface.max->setMaximum( std::numeric_limits<double>::max() );

		m_Interface.offset->setValue( image->offset );
		m_Interface.scaling->setValue( image->scaling );
		std::pair<double, double> minMax = getMinMaxFromScalingOffset( std::make_pair<double, double>(
											   image->scaling, image->offset ), image );
		m_Interface.min->setValue( minMax.first );
		m_Interface.max->setValue( minMax.second );

		m_Interface.offset->setSingleStep( image->extent / 100 );
	}
}

void ScalingWidget::maxChanged( double max )
{
	boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
	image->getPropMap().setPropertyAs<double>( "scalingMaxValue", max );
	setScalingOffset( getScalingOffsetFromMinMax( std::make_pair<double, double>( image->getPropMap().getPropertyAs<double>( "scalingMinValue" ), max ), image ), image );
}


void ScalingWidget::minChanged( double min )
{
	boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
	image->getPropMap().setPropertyAs<double>( "scalingMinValue", min );
	setScalingOffset( getScalingOffsetFromMinMax( std::make_pair<double, double>( min, image->getPropMap().getPropertyAs<double>( "scalingMaxValue" ) ), image ), image );
}

void ScalingWidget::offsetChanged( double offset )
{
	boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
	image->getPropMap().setPropertyAs<double>( "offset", offset );
	setMinMax( getMinMaxFromScalingOffset( std::make_pair<double, double>( image->scaling, offset ), image ), image );
}

void ScalingWidget::scalingChanged( double scaling )
{
	boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
	image->scaling = scaling ;
	setMinMax( getMinMaxFromScalingOffset( std::make_pair<double, double>( scaling, image->offset ), image ), image );

}

void ScalingWidget::setMinMax( std::pair< double, double > minMax, boost::shared_ptr<ImageHolder> image )
{
	disconnect( m_Interface.min, SIGNAL( valueChanged( double ) ), this, SLOT( minChanged( double ) ) );
	disconnect( m_Interface.max, SIGNAL( valueChanged( double ) ), this, SLOT( maxChanged( double ) ) );
	m_Interface.min->setValue( minMax.first ) ;
	m_Interface.max->setValue( minMax.second ) ;
	connect( m_Interface.min, SIGNAL( valueChanged( double ) ), this, SLOT( minChanged( double ) ) );
	connect( m_Interface.max, SIGNAL( valueChanged( double ) ), this, SLOT( maxChanged( double ) ) );
	image->getPropMap().setPropertyAs<double>( "scalingMinValue", minMax.first );
	image->getPropMap().setPropertyAs<double>( "scalingMaxValue", minMax.second );
	m_ViewerCore->updateScene();
}

void ScalingWidget::setScalingOffset( std::pair< double, double > scalingOffset, boost::shared_ptr< ImageHolder > image )
{
	disconnect( m_Interface.scaling, SIGNAL( valueChanged( double ) ), this, SLOT( scalingChanged( double ) ) );
	disconnect( m_Interface.offset, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged( double ) ) );
	m_Interface.scaling->setValue( scalingOffset.first );
	m_Interface.offset->setValue( scalingOffset.second );
	connect( m_Interface.scaling, SIGNAL( valueChanged( double ) ), this, SLOT( scalingChanged( double ) ) );
	connect( m_Interface.offset, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged( double ) ) );
	image->scaling = scalingOffset.first;
	image->offset = scalingOffset.second;
	synchronize();
	m_ViewerCore->updateScene();

}


void ScalingWidget::autoScale()
{
	boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
	std::pair<double, double> scalingOffset = image->getOptimalScalingToForType<isis::viewer::InternalImageType>( std::make_pair<double,double>(0.03,0.03) ) ;
	setScalingOffset( scalingOffset, image );
	setMinMax( getMinMaxFromScalingOffset( scalingOffset, image ), image );
	m_ViewerCore->updateScene();

}

void ScalingWidget::reset()
{
	boost::shared_ptr<ImageHolder> image = m_ViewerCore->getCurrentImage();
	image->offset = 0.0;
	image->scaling = 1.0;
	image->getPropMap().setPropertyAs<double>( "scalingMinValue", image->minMax.first->as<double>() );
	image->getPropMap().setPropertyAs<double>( "scalingMaxValue", image->minMax.second->as<double>() );
	synchronize();
	m_ViewerCore->updateScene();
}

std::pair< double, double > ScalingWidget::getMinMaxFromScalingOffset( const std::pair< double, double >& scalingOffset, boost::shared_ptr<ImageHolder> image )
{
	std::pair<double, double> retMinMax;
	retMinMax.first = image->minMax.first->as<double>() + scalingOffset.second;
	retMinMax.second = image->extent / scalingOffset.first + retMinMax.first;
	return retMinMax;
}

std::pair< double, double > ScalingWidget::getScalingOffsetFromMinMax( const std::pair< double, double >& minMax, boost::shared_ptr<ImageHolder> image )
{
	std::pair<double, double> retScalingOffset;
	retScalingOffset.second = minMax.first - image->minMax.first->as<double>();
	retScalingOffset.first =  image->extent / ( minMax.second - minMax.first );
	return retScalingOffset;

}
void ScalingWidget::keyPressEvent( QKeyEvent *event )
{
	if( event->key() == Qt::Key_Escape ) {
		showMe( false );
	}
}

void ScalingWidget::showMe( bool visible )
{
	setVisible( visible );
	move( QCursor::pos().x(), QCursor::pos().y() );
	m_ViewerCore->getUI()->getMainWindow()->getUI().actionShow_scaling_option->setChecked( visible );

	if( visible ) {
		synchronize();
	}
}




}
}
}