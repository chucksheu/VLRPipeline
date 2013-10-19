/*
 * VocabMatch.cpp
 *
 *  Created on: Oct 11, 2013
 *      Author: andresf
 */

#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sys/stat.h>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include <opencv2/core/core.hpp>

#include <VocabTree.h>

#include <FileUtils.hpp>

double mytime;

//int BasifyFilename(const char *filename, char *base);
void BasifyFilename(const std::string& filename, std::string& base);

void PrintHTMLHeader(FILE *f, int num_nns);

void PrintHTMLRow(FILE *f, const std::string &query, cv::Mat& scores,
		cv::Mat& perm, int num_nns, const std::vector<std::string> &db_images);

void PrintHTMLFooter(FILE *f);

int main(int argc, char **argv) {

	if (argc < 6 || argc > 8) {
		printf("\nUsage:\n"
				"\t%s <in.tree> <in.db.list> <in.query.list>"
				" <num_nbrs> <matches.out> [results.html] [candidates.txt]\n\n",
				argv[0]);
		return EXIT_FAILURE;
	}

	char *tree_in = argv[1];
	char *db_list_in = argv[2];
	char *query_list_in = argv[3];
	uint num_nbrs = atoi(argv[4]);
	char *matches_out = argv[5];
	const char *output_html = "results.html";
	const char *candidates_out = "candidates.txt";

	if (argc >= 7)
		output_html = argv[6];

	if (argc >= 8)
		candidates_out = argv[7];

	// Verifying input parameters

	boost::regex expression("^(.+)(\\.)(yaml|xml)(\\.)(gz)$");

	if (boost::regex_match(std::string(tree_in), expression) == false) {
		fprintf(stderr,
				"Input tree file must have the extension .yaml.gz or .xml.gz\n");
		return EXIT_FAILURE;
	}

	// Step 1/4: load tree

	cvflann::VocabTree tree;

	printf("-- Reading tree from [%s]\n", tree_in);

	mytime = cv::getTickCount();
	tree.load(std::string(tree_in));
	mytime = ((double) cv::getTickCount() - mytime) / cv::getTickFrequency()
			* 1000;
	printf("   Tree loaded in [%lf] ms, got [%lu] words \n", mytime,
			tree.size());

	// Step 2/4: read the database keyfiles
	printf("-- Loading DB keyfiles names and landmark id's\n");
	std::vector<std::string> db_filenames;
	std::vector<int> db_landmarks;
	std::ifstream keysList(db_list_in, std::fstream::in);

	if (keysList.is_open() == false) {
		fprintf(stderr, "Error opening file [%s] for reading\n", db_list_in);
		return EXIT_FAILURE;
	}

	// Loading file names in list into a vector
	std::string line;
	while (getline(keysList, line)) {

		// Verifying line format
		if (boost::regex_match(line, boost::regex("^(.+)\\s(.+)$")) == false) {
			fprintf(stderr,
					"Error while parsing DB list file [%s], line [%s] should be: <key.file> <landmark.id>\n",
					db_list_in, line.c_str());
			return EXIT_FAILURE;
		}

		// Extracting filename and landmark from line
		char filename[256];
		int landmark;
		sscanf(line.c_str(), "%s %d", filename, &landmark);

		// Checking that file exists, if not print error and exit
		struct stat buffer;
		if (stat(filename, &buffer) != 0) {
			fprintf(stderr, "Keypoints file [%s] doesn't exist\n", filename);
			return EXIT_FAILURE;
		}

		// Checking that filename refers to a compressed yaml or xml file
		if (boost::regex_match(std::string(filename), expression) == false) {
			fprintf(stderr,
					"Keypoints file [%s] must have the extension .yaml.gz or .xml.gz\n",
					filename);
			return EXIT_FAILURE;
		}

		db_filenames.push_back(std::string(filename));
		db_landmarks.push_back(landmark);
	}
	// Close file
	keysList.close();

	// Step 3/4: read the query keyfiles
	printf("-- Loading query keyfiles names\n");
	std::vector<std::string> query_filenames;
	keysList.open(query_list_in, std::fstream::in);

	if (keysList.is_open() == false) {
		fprintf(stderr, "Error opening file [%s] for reading\n", db_list_in);
		return EXIT_FAILURE;
	}

	// Loading file names in list into a vector
	while (getline(keysList, line)) {

		// Checking that file exists, if not print error and exit
		struct stat buffer;
		if (stat(line.c_str(), &buffer) != 0) {
			fprintf(stderr, "Keypoints file [%s] doesn't exist\n",
					line.c_str());
			return EXIT_FAILURE;
		}

		// Checking that line refers to a compressed yaml or xml file
		if (boost::regex_match(line, expression) == false) {
			fprintf(stderr,
					"Keypoints file [%s] must have the extension .yaml.gz or .xml.gz\n",
					line.c_str());
			return EXIT_FAILURE;
		}

		query_filenames.push_back(line);
	}
	// Close file
	keysList.close();

	// Step 4/4: score each query keyfile
	int normType = cv::NORM_L1;

	printf("-- Scoring [%lu] query images against [%lu] DB images using [%s]\n",
			query_filenames.size(), db_filenames.size(),
			normType == cv::NORM_L1 ? "L1-norm" :
			normType == cv::NORM_L2 ? "L2-norm" : "UNKNOWN-norm");

	std::vector<cv::KeyPoint> imgKeypoints;
	cv::Mat imgDescriptors;
	cv::Mat scores;

	int max_ld = *std::max_element(db_landmarks.begin(), db_landmarks.end());

	FILE *f_match = fopen(matches_out, "w");
	if (f_match == NULL) {
		fprintf(stderr, "Error opening file [%s] for writing\n",
				candidates_out);
		return EXIT_FAILURE;
	}

	FILE *f_candidates = fopen(candidates_out, "w");
	if (f_candidates == NULL) {
		fprintf(stderr, "Error opening file [%s] for writing\n",
				candidates_out);
		return EXIT_FAILURE;
	}

	FILE *f_html = fopen(output_html, "w");
	PrintHTMLHeader(f_html, num_nbrs);
	if (f_html == NULL) {
		fprintf(stderr, "Error opening file [%s] for writing\n",
				candidates_out);
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < query_filenames.size(); i++) {
		// Initialize keypoints and descriptors
		imgKeypoints.clear();
		imgDescriptors = cv::Mat();

		// Load query keypoints and descriptors
		FileUtils::loadFeatures(query_filenames[i], imgKeypoints,
				imgDescriptors);

		// Score query bow vector against DB images bow vectors
		mytime = cv::getTickCount();
		try {
			tree.scoreQuery(imgDescriptors, scores, db_filenames.size(),
					cv::NORM_L1);
		} catch (const std::runtime_error& error) {
			fprintf(stderr, "%s\n", error.what());
			return EXIT_FAILURE;
		}
		mytime = ((double) cv::getTickCount() - mytime) / cv::getTickFrequency()
				* 1000;

		// Print to standard output the matching scores between
		// the query bow vector and the DB images bow vectors
		for (size_t j = 0; (int) j < scores.cols; j++) {
			printf(
					"   Match score between [%lu] query image and [%lu] DB image: %f\n",
					i, j, scores.at<float>(0, j));
		}

		// Obtain indices of ordered scores
		cv::Mat perm;
		// Note: recall that the index of the images in the inverted file corresponds
		// to the zero-based line number in the file used to build the DB.
		// Hence scores matrix and db_landmarks and db_filenames vectors.
		// are equally ordered.
		// Also the images in list_db and list_db_ld must be equally ordered,
		// that implies same number of elements.
		//
		// list_db      list_db_ld
		//   img1  --->  img1 ld1
		//   img2  --->  img2 ld1
		//   img3  --->  img3 ld1
		//   img4  --->  img4 ld2
		//   img5  --->  img5 ld2
		//   img6  --->  img6 ld2
		cv::sortIdx(scores, perm, cv::SORT_EVERY_ROW + cv::SORT_DESCENDING);

		int top = MIN (num_nbrs, db_filenames.size());

		// Initialize votes vector
		// Note: size is maximum landmark id plus one because landmark index its zero-based
		std::vector<int> votes(max_ld + 1, 0);

		// Accumulating landmark votes for the top scored images
		// Note: recall that images might refer to the same landmark
		for (size_t i = 0; (int) i < top; i++) {
			votes[db_landmarks[perm.at<int>(0, i)]]++;
		}

		// Finding max voted landmark and the number of votes it got
		int max_votes = 0;
		int max_landmark = -1;
		for (int j = 0; j < max_ld + 1; j++) {
			if (votes[j] > max_votes) {
				max_votes = votes[j];
				max_landmark = j;
			}
		}

		// Print to a file the ranked list of candidates ordered by score
		fprintf(f_candidates, "%s", query_filenames[i].c_str());
		for (int j = 0; j < top; j++) {
			std::string d_base = db_filenames[perm.at<int>(0, j)];
			fprintf(f_candidates, " %s", d_base.c_str());
		}
		fprintf(f_candidates, "\n");
		fflush(f_candidates);

		// Print to a file the max voted landmark information
		fprintf(f_match, "%lu %d %d\n", i, max_landmark, max_votes);
		fflush(f_match);
		fflush(stdout);

		// Print to a file the ranked list of candidates ordered by score in HTML format
		PrintHTMLRow(f_html, query_filenames[i], scores, perm, top,
				db_filenames);
	}

	fclose(f_candidates);
	fclose(f_match);
	PrintHTMLFooter(f_html);
	fclose(f_html);

	return EXIT_SUCCESS;
}

void PrintHTMLHeader(FILE *f, int num_nns) {
	fprintf(f, "<html>\n"
			"<header>\n"
			"<title>Vocabulary tree results</title>\n"
			"</header>\n"
			"<body>\n"
			"<h1>Vocabulary tree results</h1>\n"
			"<hr>\n\n");

	fprintf(f, "<table border=2 align=center>\n<tr>\n<th>Query image</th>\n");
	for (int i = 0; i < num_nns; i++) {
		fprintf(f, "<th>Match %d</th>\n", i + 1);
	}
	fprintf(f, "</tr>\n");
}

void PrintHTMLRow(FILE *f, const std::string &query, cv::Mat& scores,
		cv::Mat& perm, int num_nns, const std::vector<std::string> &db_images) {

	std::string q_thumb;

	BasifyFilename(query, q_thumb);

	fprintf(f,
			"<tr align=center>\n<td><img src=\"%s\" style=\"max-height:200px\"><br><p>%s</p></td>\n",
			q_thumb.c_str(), q_thumb.c_str());

	for (int i = 0; i < num_nns; i++) {
		std::string d_thumb;
		BasifyFilename(db_images[perm.at<int>(0, i)], d_thumb);

		fprintf(f,
				"<td><img src=\"%s\" style=\"max-height:200px\"><br><p>%s</p></td>\n",
				d_thumb.c_str(), d_thumb.c_str());
	}

	fprintf(f, "</tr>\n<tr align=right>\n");

	fprintf(f, "<td></td>\n");
	for (int i = 0; i < num_nns; i++)
		fprintf(f, "<td>%0.5f</td>\n", scores.at<float>(0, perm.at<int>(0, i)));

	fprintf(f, "</tr>\n");
}

void PrintHTMLFooter(FILE *f) {
	fprintf(f, "</tr>\n"
			"</table>\n"
			"<hr>\n"
			"</body>\n"
			"</html>\n");
}

//int BasifyFilename(const char *filename, char *base)
void BasifyFilename(const std::string& key_fname, std::string& img_fname) {

	// Extract key file name from query key file path
	std::vector<std::string> tokens;
	boost::split(tokens, key_fname, boost::is_any_of("//"));
	CV_Assert(tokens.size() > 0);

	// Get query key file name and query images path
	std::string query_fname = tokens.back();
	tokens.pop_back();
	img_fname = boost::algorithm::join(tokens, "/");
	tokens.clear();
	boost::split(tokens, query_fname, boost::is_any_of("\\."));
	CV_Assert(tokens.size() == 4);

	img_fname += std::string("/") + tokens[0] + std::string(".thumb.jpg");
}
