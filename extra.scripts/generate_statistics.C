/*
 * the format to prepare the root file is:
 * "channel/I;pedestal/D;sigma_raw/D;sigma/D;bad_channel/D;device_type/C;device_location/C;device_kind/C;device_code/I;device_connector/C;test_kind/C;date/C"
 */
#define d_skynet_X 2000
#define d_skynet_Y 800
#define d_skynet_string_branch_size 3
#define d_skynet_string_size 256
typedef struct s_skynet_quality_data {
	int channel, device_code, bad_channel;
	double pedestal, sigma_raw, sigma;
	char device_type[d_skynet_string_branch_size], device_location[d_skynet_string_branch_size], test_kind[d_skynet_string_branch_size];
} s_skynet_quality_data;
typedef struct s_skynet_plot {
	char name[d_skynet_string_size], title[d_skynet_string_size];
	TCanvas *canvas;
	TH1F *plot;
} s_skynet_plot;
TCanvas *p_skynet_draw_quality_canvas(const char *name, const char *title) {
	TCanvas *result = NULL;
	if ((result = new TCanvas(name, title, d_skynet_X, d_skynet_Y))) {
		result->SetFillColor(0);
		result->SetBorderMode(0);
		result->SetFrameBorderMode(0);
	}
	return result;
}

struct s_skynet_plot *p_skynet_draw_quality_plot(const char *name, const char *title, float low_x, float high_x) {
	struct s_skynet_plot *result = NULL;
	if ((result = (struct s_skynet_plot *) malloc(sizeof(struct s_skynet_plot)))) {
		strncpy(result->name, name, d_skynet_string_size);
		strncpy(result->title, title, d_skynet_string_size);
		if ((result->plot = new TH1F(name, title, (high_x-low_x)*4, low_x, high_x))) {
			result->plot->SetStats(kTRUE);
			gStyle->SetOptStat("emro");
			result->plot->SetLineColor(kRed);
			result->plot->SetLineWidth(2.0f);
			result->plot->SetFillColor(kRed);
			result->plot->SetFillStyle(3005);
		}
	}
	return result;
}

void f_skynet_draw_quality(const char *file) {
	struct s_skynet_quality_data current_node;
	char name[d_skynet_string_size];
	/* prepare canvas */
	struct s_skynet_plot *plots[] = {
		p_skynet_draw_quality_plot("ped_LLPG",	 	"Pedestal PG Ladder (test 'l')",	0, 500), 	/* 0 */
		p_skynet_draw_quality_plot("ped_LLGV", 		"Pedestal GV Ladder (test 'l')", 	0, 500), 	/* 1 */
		p_skynet_draw_quality_plot("ped_LLPGGV", 	"Pedestal PG+GV Ladder (test 'l')", 	0, 500), 	/* 2 */
		p_skynet_draw_quality_plot("sraw_LLPG", 	"Sigma Raw PG Ladder (test 'l')", 	0, 50), 	/* 3 */
		p_skynet_draw_quality_plot("sraw_LLGV", 	"Sigma Raw  GV Ladder (test 'l')", 	0, 50),		/* 4 */
		p_skynet_draw_quality_plot("sraw_LLPGGV", 	"Sigma Raw PG+GV Ladder (test 'l')",	0, 50),		/* 5 */
		p_skynet_draw_quality_plot("sig_LLPG", 		"Sigma PG Ladder (test 'l')", 		0, 50),		/* 6 */
		p_skynet_draw_quality_plot("sig_LLGV", 		"Sigma GV Ladder (test 'l')", 		0, 50),		/* 7 */
		p_skynet_draw_quality_plot("sig_LLPGGV", 	"Sigma PG+GV Ladder (test 'l')", 	0, 50),		/* 8 */
		p_skynet_draw_quality_plot("ped_LMPG",	 	"Pedestal PG Ladder (test 'm')", 	0, 500), 	/* 9 */
		p_skynet_draw_quality_plot("ped_LMGV", 		"Pedestal GV Ladder (test 'm')", 	0, 500), 	/* 10 */
		p_skynet_draw_quality_plot("ped_LMPGGV", 	"Pedestal PG+GV Ladder (test 'm')", 	0, 500), 	/* 11 */
		p_skynet_draw_quality_plot("sraw_LMPG", 	"Sigma Raw PG Ladder (test 'm')", 	0, 50), 	/* 12 */
		p_skynet_draw_quality_plot("sraw_LMGV", 	"Sigma Raw  GV Ladder (test 'm')", 	0, 50),		/* 13 */
		p_skynet_draw_quality_plot("sraw_LMPGGV", 	"Sigma Raw PG+GV Ladder (test 'm')", 	0, 50),		/* 14 */
		p_skynet_draw_quality_plot("sig_LMPG", 		"Sigma PG Ladder (test 'm')", 		0, 50),		/* 15 */
		p_skynet_draw_quality_plot("sig_LMGV", 		"Sigma GV Ladder (test 'm')", 		0, 50),		/* 16 */
		p_skynet_draw_quality_plot("sig_LMPGGV", 	"Sigma PG+GV Ladder (test 'm')", 	0, 50),		/* 17 */
		p_skynet_draw_quality_plot("ped_HB", 		"Pedestal Hybrid (test 'b')", 		0, 500), 	/* 18 */
		p_skynet_draw_quality_plot("ped_HB_4096",	"Pedestal Hybrid (test 'b')",		0, 4096),	/* 19 */
		p_skynet_draw_quality_plot("sraw_HB",		"Sigma Raw Hybrid (test 'b')",		0, 50),		/* 20 */
		p_skynet_draw_quality_plot("sig_HB",		"Sigma Hybrid (test 'b')",		0, 50),		/* 21 */
		NULL
	};
	int entries, index;
	TFile *stream = new TFile(file, "read");
	TTree *tree;
	if (stream->IsOpen()) {
		/* here we have to have myTree */
		if ((tree = (TTree *)stream->Get("myTree"))) {
			if ((entries = tree->GetEntries())) {
				/* prepare interface */
				tree->SetBranchAddress("channel", &current_node.channel);
				tree->SetBranchAddress("pedestal", &current_node.pedestal);
				tree->SetBranchAddress("sigma_raw", &current_node.sigma_raw);
				tree->SetBranchAddress("sigma", &current_node.sigma);
				tree->SetBranchAddress("bad_channel", &current_node.bad_channel);
				tree->SetBranchAddress("device_type", current_node.device_type);
				tree->SetBranchAddress("device_location", current_node.device_location);
				tree->SetBranchAddress("device_code", &current_node.device_code);
				tree->SetBranchAddress("test_kind", current_node.test_kind);
				for (index = 0; index < entries; ++index) {
					tree->GetEntry(index);
					if (current_node.device_type[0] == 'L') {
						if (current_node.test_kind[0] == 'l') {
							if (current_node.device_location[0] == 'P') {
								plots[0]->plot->Fill(current_node.pedestal);
								plots[3]->plot->Fill(current_node.sigma_raw);
								plots[6]->plot->Fill(current_node.sigma);
							} else if (current_node.device_location[0] == 'G') {
								plots[1]->plot->Fill(current_node.pedestal);
								plots[4]->plot->Fill(current_node.sigma_raw);
								plots[7]->plot->Fill(current_node.sigma);
							}
							plots[2]->plot->Fill(current_node.pedestal);
							plots[5]->plot->Fill(current_node.sigma_raw);
							plots[8]->plot->Fill(current_node.sigma);
						} else if (current_node.test_kind[0] == 'm') {
							if (current_node.device_location[0] == 'P') {
								plots[9]->plot->Fill(current_node.pedestal);
								plots[12]->plot->Fill(current_node.sigma_raw);
								plots[15]->plot->Fill(current_node.sigma);
							} else if (current_node.device_location[0] == 'G') {
								plots[10]->plot->Fill(current_node.pedestal);
								plots[13]->plot->Fill(current_node.sigma_raw);
								plots[16]->plot->Fill(current_node.sigma);
							}
							plots[11]->plot->Fill(current_node.pedestal);
							plots[14]->plot->Fill(current_node.sigma_raw);
							plots[17]->plot->Fill(current_node.sigma);
						}
					} else if ((current_node.device_type[0] == 'H') && (current_node.test_kind[0] == 'b')) {
						plots[18]->plot->Fill(current_node.pedestal);
						plots[19]->plot->Fill(current_node.pedestal);
						plots[20]->plot->Fill(current_node.sigma_raw);
						plots[21]->plot->Fill(current_node.sigma);
					}
				}
				for (index = 0; plots[index]; ++index)
					if ((plots[index]->canvas = p_skynet_draw_quality_canvas(plots[index]->name, plots[index]->title))) {
						//plots[index]->canvas->SetLogy();
						plots[index]->plot->Draw();
						plots[index]->canvas->Update();
						snprintf(name, d_skynet_string_size, "%s_%s.pdf", file, plots[index]->name);
						plots[index]->canvas->Print(name);
					}
			}
			stream->Close();
		}
	}
}
