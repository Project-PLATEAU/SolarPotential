using System;
using System.Windows.Forms;
using SolarPotential._2.Dialog;

namespace SolarPotential
{
    public partial class dlg_Aggregating : Form
    {
        public dlg_Aggregating()
        {
            InitializeComponent();

            //初期設定
            Initial();
        }

        // 初期設定
        private void Initial()
        {
            btn_OK.Visible = false;
            btn_OK.Enabled = false;
            btn_Cancel.Visible = true;
            btn_Cancel.Enabled = true;
        }

        // キャンセルボタン
        private void btn_Cancel_Click(object sender, EventArgs e)
        {
            // 処理中ダイアログ表示
            dlg_CancelConfirm dlgCancelConfirm = new dlg_CancelConfirm();
            dlgCancelConfirm.label1.Text = "集計処理をキャンセルします。よろしいですか？";
            dlgCancelConfirm.ShowDialog();

            if (dlgCancelConfirm.DialogResult == System.Windows.Forms.DialogResult.OK)
            {
                // キャンセル用テキストを出力フォルダ直下に作成

                // メインフォームの実行を停止
                if (frm_Aggregate.ParentInstance.taskCanceler != null) frm_Aggregate.ParentInstance.taskCanceler.Cancel();
                dlgCancelConfirm.Close();
                this.Close();
            }
            else
            {
                dlgCancelConfirm.Close();
            }
        }
        // OKボタン
        private void btn_OK_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        // タイマー処理
        private void tim_Aggregate_Imitation_Tick(object sender, EventArgs e)
        {
            btn_OK.Visible = true;
            btn_OK.Enabled = true;
            btn_Cancel.Visible = false;
            btn_Cancel.Enabled = false;
            label1.Text = "集計処理が終了しました。";
        }
    }
}
