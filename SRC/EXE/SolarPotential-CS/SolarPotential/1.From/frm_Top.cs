using System;
using System.Windows.Forms;

namespace SolarPotential
{
    public partial class frm_Top : Form
    {
        public frm_Top()
        {
            InitializeComponent();
        }
        // 終了ボタン
        private void btn_End_Click(object sender, EventArgs e)
        {
            dlg_EndConfirm dlgEndConfirm = new dlg_EndConfirm();
            dlgEndConfirm.ShowDialog();

            if (dlgEndConfirm.DialogResult == System.Windows.Forms.DialogResult.OK)
            {
                this.Close();
                Application.Exit();
            }
        }
        // 解析ボタン
        private void btn_Analyze_Click(object sender, EventArgs e)
        {
            frm_Analyze frmAnalyze = new frm_Analyze();
            frmAnalyze.ShowDialog();
        }
        // 集計ボタン
        private void btn_Aggregate_Click(object sender, EventArgs e)
        {
            frm_Aggregate frmAggregate = new frm_Aggregate();
            frmAggregate.ShowDialog();
        }
    }
}
