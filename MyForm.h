#pragma once

namespace hands_viewer {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Panel^  panel1;
	protected:
	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::CheckBox^  IDC_PARAM;
	private: System::Windows::Forms::CheckBox^  IDC_GENODE;
	private: System::Windows::Forms::PictureBox^  IDC_PANEL1;
	private: System::Windows::Forms::RichTextBox^  IDC_INFOBOX;
	private: System::Windows::Forms::Panel^  panel2;
	private: System::Windows::Forms::RichTextBox^  IDC_MOCAPDATA;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::CheckBox^  IDC_POSITIONWORLD;
	private: System::Windows::Forms::CheckBox^  IDC_POSITIONLOCAL;
	private: System::Windows::Forms::CheckBox^  IDC_ROTATIONLOCAL;
	private: System::Windows::Forms::CheckBox^  IDC_ROTATIONGLOBAL;
	private: System::Windows::Forms::Button^  IDC_CHECKALL;
	private: System::Windows::Forms::Button^  IDC_SNAPSHOTSAVE;
	private: System::Windows::Forms::Button^  IDC_SNAPSHOT;
	private: System::Windows::Forms::Button^  IDC_UNCHECKALL;








	protected:


	protected:

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MyForm::typeid));
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->IDC_INFOBOX = (gcnew System::Windows::Forms::RichTextBox());
			this->IDC_PANEL1 = (gcnew System::Windows::Forms::PictureBox());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->IDC_GENODE = (gcnew System::Windows::Forms::CheckBox());
			this->IDC_PARAM = (gcnew System::Windows::Forms::CheckBox());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->IDC_SNAPSHOTSAVE = (gcnew System::Windows::Forms::Button());
			this->IDC_SNAPSHOT = (gcnew System::Windows::Forms::Button());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->IDC_UNCHECKALL = (gcnew System::Windows::Forms::Button());
			this->IDC_CHECKALL = (gcnew System::Windows::Forms::Button());
			this->IDC_ROTATIONLOCAL = (gcnew System::Windows::Forms::CheckBox());
			this->IDC_ROTATIONGLOBAL = (gcnew System::Windows::Forms::CheckBox());
			this->IDC_POSITIONLOCAL = (gcnew System::Windows::Forms::CheckBox());
			this->IDC_POSITIONWORLD = (gcnew System::Windows::Forms::CheckBox());
			this->IDC_MOCAPDATA = (gcnew System::Windows::Forms::RichTextBox());
			this->panel1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->IDC_PANEL1))->BeginInit();
			this->groupBox1->SuspendLayout();
			this->panel2->SuspendLayout();
			this->groupBox2->SuspendLayout();
			this->SuspendLayout();
			// 
			// panel1
			// 
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel1->Controls->Add(this->IDC_INFOBOX);
			this->panel1->Controls->Add(this->IDC_PANEL1);
			this->panel1->Controls->Add(this->groupBox1);
			this->panel1->Location = System::Drawing::Point(32, 20);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(399, 483);
			this->panel1->TabIndex = 0;
			// 
			// IDC_INFOBOX
			// 
			this->IDC_INFOBOX->Location = System::Drawing::Point(19, 285);
			this->IDC_INFOBOX->Name = L"IDC_INFOBOX";
			this->IDC_INFOBOX->Size = System::Drawing::Size(221, 161);
			this->IDC_INFOBOX->TabIndex = 2;
			this->IDC_INFOBOX->Text = L"";
			// 
			// IDC_PANEL1
			// 
			this->IDC_PANEL1->Location = System::Drawing::Point(9, 17);
			this->IDC_PANEL1->Name = L"IDC_PANEL1";
			this->IDC_PANEL1->Size = System::Drawing::Size(231, 180);
			this->IDC_PANEL1->TabIndex = 1;
			this->IDC_PANEL1->TabStop = false;
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->IDC_GENODE);
			this->groupBox1->Controls->Add(this->IDC_PARAM);
			this->groupBox1->Location = System::Drawing::Point(240, 20);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(139, 178);
			this->groupBox1->TabIndex = 0;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Viewer Options";
			// 
			// IDC_GENODE
			// 
			this->IDC_GENODE->AutoSize = true;
			this->IDC_GENODE->Location = System::Drawing::Point(17, 46);
			this->IDC_GENODE->Name = L"IDC_GENODE";
			this->IDC_GENODE->Size = System::Drawing::Size(53, 17);
			this->IDC_GENODE->TabIndex = 1;
			this->IDC_GENODE->Text = L"Joints";
			this->IDC_GENODE->UseVisualStyleBackColor = true;
			// 
			// IDC_PARAM
			// 
			this->IDC_PARAM->AutoSize = true;
			this->IDC_PARAM->Location = System::Drawing::Point(17, 23);
			this->IDC_PARAM->Name = L"IDC_PARAM";
			this->IDC_PARAM->Size = System::Drawing::Size(68, 17);
			this->IDC_PARAM->TabIndex = 0;
			this->IDC_PARAM->Text = L"Skeleton";
			this->IDC_PARAM->UseVisualStyleBackColor = true;
			// 
			// panel2
			// 
			this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel2->Controls->Add(this->IDC_SNAPSHOTSAVE);
			this->panel2->Controls->Add(this->IDC_SNAPSHOT);
			this->panel2->Controls->Add(this->groupBox2);
			this->panel2->Controls->Add(this->IDC_MOCAPDATA);
			this->panel2->Location = System::Drawing::Point(466, 20);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(445, 483);
			this->panel2->TabIndex = 1;
			// 
			// IDC_SNAPSHOTSAVE
			// 
			this->IDC_SNAPSHOTSAVE->Location = System::Drawing::Point(140, 290);
			this->IDC_SNAPSHOTSAVE->Name = L"IDC_SNAPSHOTSAVE";
			this->IDC_SNAPSHOTSAVE->Size = System::Drawing::Size(108, 23);
			this->IDC_SNAPSHOTSAVE->TabIndex = 3;
			this->IDC_SNAPSHOTSAVE->Text = L"Save a Snapshot";
			this->IDC_SNAPSHOTSAVE->UseVisualStyleBackColor = true;
			// 
			// IDC_SNAPSHOT
			// 
			this->IDC_SNAPSHOT->Location = System::Drawing::Point(140, 261);
			this->IDC_SNAPSHOT->Name = L"IDC_SNAPSHOT";
			this->IDC_SNAPSHOT->Size = System::Drawing::Size(108, 23);
			this->IDC_SNAPSHOT->TabIndex = 2;
			this->IDC_SNAPSHOT->Text = L"Take a Snapshot";
			this->IDC_SNAPSHOT->UseVisualStyleBackColor = true;
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->IDC_UNCHECKALL);
			this->groupBox2->Controls->Add(this->IDC_CHECKALL);
			this->groupBox2->Controls->Add(this->IDC_ROTATIONLOCAL);
			this->groupBox2->Controls->Add(this->IDC_ROTATIONGLOBAL);
			this->groupBox2->Controls->Add(this->IDC_POSITIONLOCAL);
			this->groupBox2->Controls->Add(this->IDC_POSITIONWORLD);
			this->groupBox2->Location = System::Drawing::Point(299, 22);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(141, 195);
			this->groupBox2->TabIndex = 1;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Gesture Parameters";
			// 
			// IDC_UNCHECKALL
			// 
			this->IDC_UNCHECKALL->Location = System::Drawing::Point(31, 166);
			this->IDC_UNCHECKALL->Name = L"IDC_UNCHECKALL";
			this->IDC_UNCHECKALL->Size = System::Drawing::Size(75, 23);
			this->IDC_UNCHECKALL->TabIndex = 5;
			this->IDC_UNCHECKALL->Text = L"Uncheck All";
			this->IDC_UNCHECKALL->UseVisualStyleBackColor = true;
			// 
			// IDC_CHECKALL
			// 
			this->IDC_CHECKALL->Location = System::Drawing::Point(31, 136);
			this->IDC_CHECKALL->Name = L"IDC_CHECKALL";
			this->IDC_CHECKALL->Size = System::Drawing::Size(75, 23);
			this->IDC_CHECKALL->TabIndex = 4;
			this->IDC_CHECKALL->Text = L"Check All";
			this->IDC_CHECKALL->UseVisualStyleBackColor = true;
			// 
			// IDC_ROTATIONLOCAL
			// 
			this->IDC_ROTATIONLOCAL->AutoSize = true;
			this->IDC_ROTATIONLOCAL->Location = System::Drawing::Point(21, 93);
			this->IDC_ROTATIONLOCAL->Name = L"IDC_ROTATIONLOCAL";
			this->IDC_ROTATIONLOCAL->Size = System::Drawing::Size(95, 17);
			this->IDC_ROTATIONLOCAL->TabIndex = 3;
			this->IDC_ROTATIONLOCAL->Text = L"Local Rotation";
			this->IDC_ROTATIONLOCAL->UseVisualStyleBackColor = true;
			// 
			// IDC_ROTATIONGLOBAL
			// 
			this->IDC_ROTATIONGLOBAL->AutoSize = true;
			this->IDC_ROTATIONGLOBAL->Location = System::Drawing::Point(21, 72);
			this->IDC_ROTATIONGLOBAL->Name = L"IDC_ROTATIONGLOBAL";
			this->IDC_ROTATIONGLOBAL->Size = System::Drawing::Size(99, 17);
			this->IDC_ROTATIONGLOBAL->TabIndex = 2;
			this->IDC_ROTATIONGLOBAL->Text = L"Global Rotation";
			this->IDC_ROTATIONGLOBAL->UseVisualStyleBackColor = true;
			// 
			// IDC_POSITIONLOCAL
			// 
			this->IDC_POSITIONLOCAL->AutoSize = true;
			this->IDC_POSITIONLOCAL->Location = System::Drawing::Point(21, 39);
			this->IDC_POSITIONLOCAL->Name = L"IDC_POSITIONLOCAL";
			this->IDC_POSITIONLOCAL->Size = System::Drawing::Size(95, 17);
			this->IDC_POSITIONLOCAL->TabIndex = 1;
			this->IDC_POSITIONLOCAL->Text = L"Image Position";
			this->IDC_POSITIONLOCAL->UseVisualStyleBackColor = true;
			// 
			// IDC_POSITIONWORLD
			// 
			this->IDC_POSITIONWORLD->AutoSize = true;
			this->IDC_POSITIONWORLD->Location = System::Drawing::Point(21, 21);
			this->IDC_POSITIONWORLD->Name = L"IDC_POSITIONWORLD";
			this->IDC_POSITIONWORLD->Size = System::Drawing::Size(96, 17);
			this->IDC_POSITIONWORLD->TabIndex = 0;
			this->IDC_POSITIONWORLD->Text = L"Global Position";
			this->IDC_POSITIONWORLD->UseVisualStyleBackColor = true;
			// 
			// IDC_MOCAPDATA
			// 
			this->IDC_MOCAPDATA->Location = System::Drawing::Point(35, 22);
			this->IDC_MOCAPDATA->Name = L"IDC_MOCAPDATA";
			this->IDC_MOCAPDATA->Size = System::Drawing::Size(249, 195);
			this->IDC_MOCAPDATA->TabIndex = 0;
			this->IDC_MOCAPDATA->Text = L"";
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(923, 503);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->panel1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"MyForm";
			this->Text = L"Gesture Snapshot Tool";
			this->panel1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->IDC_PANEL1))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
