#include <xapian.h>

#include <cstring>
#include <string>
#include <iostream>

void usage(char **argv) {
        std::cerr << "usage: " << argv[0] <<
                     " <action> <path to database>" << std::endl;
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        usage(argv);
        return 1;
    }

    try {
        char *action = argv[1];
        char *db_path = argv[2];

        if(!strcmp(action, "index")) {
            Xapian::WritableDatabase db(db_path, Xapian::DB_CREATE_OR_OPEN);

            Xapian::TermGenerator indexer;
            Xapian::Stem stemmer("english");
            indexer.set_stemmer(stemmer);

            std::string doc_txt;
            while(true) {
                if(std::cin.eof()) break;

                std::string line;
                getline(std::cin, line);
                doc_txt += line;
            }

            if(!doc_txt.empty()) {
                Xapian::Document doc;
                doc.set_data(doc_txt);

                indexer.set_document(doc);
                indexer.index_text(doc_txt);

                db.add_document(doc);

                std::cout << "Indexed: " << indexer.get_description() << std::endl;
            }

            db.commit();
        } else if(!strcmp(action, "search")) {
            if(argc < 4) {
                std::cerr << "You must supply a query string" << std::endl;
                return 1;
            }

            Xapian::Database db(db_path);
            Xapian::Enquire enquire(db);

            std::string query_str = argv[3];
            argv+= 4;
            while(*argv) {
                query_str += ' ';
                query_str += *argv++;
            }

            Xapian::QueryParser qp;
            Xapian::Stem stemmer("english");
            qp.set_stemmer(stemmer);
            qp.set_database(db);
            qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);

            Xapian::Query query = qp.parse_query(query_str);
            std::cout << "Parsed query is: " << query.get_description() <<
                         std::endl;

            enquire.set_query(query);
            Xapian::MSet matches = enquire.get_mset(0, 10);

            std::cout << matches.get_matches_estimated() << " results found.\n";
            std::cout << "Matches 1-" << matches.size() << ":\n" << std::endl;

            for (Xapian::MSetIterator i = matches.begin();
                    i != matches.end(); ++i) {
                std::cout << i.get_rank() + 1 << ": " << i.get_percent() <<
                        "% docid=" << *i << " [" <<
                        i.get_document().get_data()<< "]" << std::endl <<
                        std::endl;
            }
        } else {
            std::cerr << "Invalid action " << action << std::endl;
            usage(argv);
            return 1;
        }

    } catch (const Xapian::Error &error) {
        std::cout << "Exception: " << error.get_msg() << std::endl;
    }
}

