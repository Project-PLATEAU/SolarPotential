
namespace SolarPotential
{
    partial class frm_Top
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(frm_Top));
            this.panel2 = new System.Windows.Forms.Panel();
            this.label2 = new System.Windows.Forms.Label();
            this.btn_Analyze = new System.Windows.Forms.Button();
            this.btn_End = new System.Windows.Forms.Button();
            this.panel3 = new System.Windows.Forms.Panel();
            this.label3 = new System.Windows.Forms.Label();
            this.btn_Aggregate = new System.Windows.Forms.Button();
            this.panel2.SuspendLayout();
            this.panel3.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel2
            // 
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel2.Controls.Add(this.label2);
            this.panel2.Controls.Add(this.btn_Analyze);
            this.panel2.Location = new System.Drawing.Point(175, 98);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(400, 450);
            this.panel2.TabIndex = 6;
            // 
            // label2
            // 
            this.label2.Font = new System.Drawing.Font("MS UI Gothic", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label2.Location = new System.Drawing.Point(35, 307);
            this.label2.MaximumSize = new System.Drawing.Size(999, 999);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(325, 100);
            this.label2.TabIndex = 3;
            this.label2.Text = "3D都市モデルを読み込み\r\n発電ポテンシャル推計や\r\n反射シミュレーション等の\r\n解析処理を行う場合はこちら";
            // 
            // btn_Analyze
            // 
            this.btn_Analyze.Image = ((System.Drawing.Image)(resources.GetObject("btn_Analyze.Image")));
            this.btn_Analyze.Location = new System.Drawing.Point(37, 37);
            this.btn_Analyze.Name = "btn_Analyze";
            this.btn_Analyze.Size = new System.Drawing.Size(325, 253);
            this.btn_Analyze.TabIndex = 2;
            this.btn_Analyze.UseVisualStyleBackColor = true;
            this.btn_Analyze.Click += new System.EventHandler(this.btn_Analyze_Click);
            // 
            // btn_End
            // 
            this.btn_End.Font = new System.Drawing.Font("MS UI Gothic", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.btn_End.Location = new System.Drawing.Point(1070, 628);
            this.btn_End.Name = "btn_End";
            this.btn_End.Size = new System.Drawing.Size(180, 40);
            this.btn_End.TabIndex = 5;
            this.btn_End.Text = "終了";
            this.btn_End.UseVisualStyleBackColor = true;
            this.btn_End.Click += new System.EventHandler(this.btn_End_Click);
            // 
            // panel3
            // 
            this.panel3.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel3.Controls.Add(this.label3);
            this.panel3.Controls.Add(this.btn_Aggregate);
            this.panel3.Location = new System.Drawing.Point(705, 98);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(400, 450);
            this.panel3.TabIndex = 7;
            // 
            // label3
            // 
            this.label3.Font = new System.Drawing.Font("MS UI Gothic", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label3.Location = new System.Drawing.Point(35, 307);
            this.label3.MaximumSize = new System.Drawing.Size(999, 999);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(325, 100);
            this.label3.TabIndex = 4;
            this.label3.Text = "解析処理後のデータを読み込み\r\n任意の範囲での解析結果の集計や\r\nパネル設置適地の条件を指定しての\r\n集計処理を行う場合はこちら";
            // 
            // btn_Aggregate
            // 
            this.btn_Aggregate.Image = ((System.Drawing.Image)(resources.GetObject("btn_Aggregate.Image")));
            this.btn_Aggregate.Location = new System.Drawing.Point(37, 37);
            this.btn_Aggregate.Name = "btn_Aggregate";
            this.btn_Aggregate.Size = new System.Drawing.Size(325, 253);
            this.btn_Aggregate.TabIndex = 3;
            this.btn_Aggregate.UseVisualStyleBackColor = true;
            this.btn_Aggregate.Click += new System.EventHandler(this.btn_Aggregate_Click);
            // 
            // frm_Top
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1264, 681);
            this.ControlBox = false;
            this.Controls.Add(this.panel2);
            this.Controls.Add(this.btn_End);
            this.Controls.Add(this.panel3);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Location = new System.Drawing.Point(1280, 720);
            this.MaximumSize = new System.Drawing.Size(1280, 720);
            this.Name = "frm_Top";
            this.Text = "カーボンニュートラル施策推進支援システム";
            this.panel2.ResumeLayout(false);
            this.panel3.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button btn_Analyze;
        private System.Windows.Forms.Button btn_End;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button btn_Aggregate;
    }
}