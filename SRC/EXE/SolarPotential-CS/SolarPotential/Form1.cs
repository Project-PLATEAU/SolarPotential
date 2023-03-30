using System.Windows.Forms;

namespace SolarPotential
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            frm_Top frmTop = new frm_Top();
            frmTop.ShowDialog();
        }
    }
}
