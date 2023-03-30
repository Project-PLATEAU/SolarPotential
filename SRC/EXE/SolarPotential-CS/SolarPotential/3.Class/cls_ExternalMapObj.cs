
namespace SolarPotential._3.Class
{
    [System.Runtime.InteropServices.ComVisibleAttribute(true)]
    public class cls_ExternalMapObj
    {
        public delegate void MoveCenterEventHandler(double lat, double lon, int zoom);
        public delegate void PickStartEventHandler(double top, double bottom, double left, double right);

        public event MoveCenterEventHandler MoveCenter;
        public event PickStartEventHandler PickStart;


        public void MoveEnd(double lat, double lon, int zoom)
        {
            MoveCenter(lat, lon, zoom);
        }

        public void GetExtent(double top, double bottom, double left, double right)
        {
            PickStart(top, bottom, left, right);
        }
    }
}
