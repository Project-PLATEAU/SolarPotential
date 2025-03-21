
namespace SolarPotential._3_Class
{
    [System.Runtime.InteropServices.ComVisibleAttribute(true)]
    public class ExternalMapObj
    {
        public delegate void MoveCenterEventHandler(double lat, double lon, int zoom);
        public delegate void PickStartEventHandler(double top, double bottom, double left, double right);
        public delegate void AddAreaEventHandler(int id, string coordinates);
        public delegate void EditAreaEventHandler(int id, string coordinates);
        public delegate void DeleteAreaEventHandler(int id);

        public event MoveCenterEventHandler MoveCenter;
        public event PickStartEventHandler PickStart;
        public event AddAreaEventHandler AddArea;
        public event EditAreaEventHandler EditArea;
        public event DeleteAreaEventHandler DeleteArea;


        public void MoveEnd(double lat, double lon, int zoom)
        {
            MoveCenter(lat, lon, zoom);
        }

        public void GetExtent(double top, double bottom, double left, double right)
        {
            PickStart(top, bottom, left, right);
        }

        public void AddExtent(int id, string coordinates)
        {
            AddArea(id, coordinates);
        }

        public void EditExtent(int id, string coordinates)
        {
            EditArea(id, coordinates);
        }

        public void DeleteExtent(int id)
        {
            DeleteArea(id);
        }
    }
}
