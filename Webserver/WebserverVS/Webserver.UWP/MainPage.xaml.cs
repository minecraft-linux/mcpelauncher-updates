using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel.Core;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Composition;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Hosting;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// Die Elementvorlage "Leere Seite" wird unter https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x407 dokumentiert.

namespace Webserver.UWP
{
    /// <summary>
    /// Eine leere Seite, die eigenständig verwendet oder zu der innerhalb eines Rahmens navigiert werden kann.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private Compositor _compositor;
        private SpriteVisual _hostSprite;
        private CoreApplicationViewTitleBar coreTitleBar;

        public MainPage()
        {
            this.InitializeComponent();
            ApplicationViewTitleBar formattableTitleBar = ApplicationView.GetForCurrentView().TitleBar;
            formattableTitleBar.ButtonBackgroundColor = Colors.Transparent;
            formattableTitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
            coreTitleBar = CoreApplication.GetCurrentView().TitleBar;
            //Titlebar.Height = coreTitleBar.Height;
            coreTitleBar.LayoutMetricsChanged += CoreTitleBar_LayoutMetricsChanged;
            coreTitleBar.ExtendViewIntoTitleBar = true;
            //ApplicationViewTitleBar formattableTitleBar = ApplicationView.GetForCurrentView().TitleBar;
            //formattableTitleBar.ButtonBackgroundColor = Colors.Transparent;
            //CoreApplicationViewTitleBar coreTitleBar = CoreApplication.GetCurrentView().TitleBar;
            //coreTitleBar.ExtendViewIntoTitleBar = true;
            //_compositor = ElementCompositionPreview.GetElementVisual(this).Compositor;
            //_hostSprite = _compositor.CreateSpriteVisual();
            //_hostSprite.Size = new System.Numerics.Vector2((float)ActualWidth, (float)ActualHeight);
            //ElementCompositionPreview.SetElementChildVisual(
            //maingrid, _hostSprite);
            //_hostSprite.Brush = _compositor.CreateHostBackdropBrush();

        }

        private void CoreTitleBar_LayoutMetricsChanged(CoreApplicationViewTitleBar sender, object args)
        {
            Titlebar.Height = sender.Height;
        }

        private void applyAcrylicAccent()
        {
            _compositor = ElementCompositionPreview.GetElementVisual(this).Compositor;
            _hostSprite = _compositor.CreateSpriteVisual();
            _hostSprite.Size = new Vector2((float)maingrid.ActualWidth, (float)maingrid.ActualHeight);

            ElementCompositionPreview.SetElementChildVisual(
                maingrid, _hostSprite);
            _hostSprite.Brush = _compositor.CreateHostBackdropBrush();
        }

        private void Page_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_hostSprite != null)
                _hostSprite.Size = e.NewSize.ToVector2();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            applyAcrylicAccent();
            
        }
    }
}
