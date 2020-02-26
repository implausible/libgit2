#include "clar_libgit2.h"

#include "git2/clone.h"
#include "git2/indexer.h"

#define SLOW_LIVE_REPO "https://github.com/nodegit/test.git"

static git_repository *g_repo;
static git_clone_options g_options;

static char *_remote_expectcontinue = NULL;

static int ssl_cert(git_cert *cert, int valid, const char *host, void *payload)
{
	GIT_UNUSED(cert);
  GIT_UNUSED(valid);
	GIT_UNUSED(host);
	GIT_UNUSED(payload);

	return 0;
}

void test_online_slow_clone__initialize(void)
{
	git_checkout_options dummy_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_fetch_options dummy_fetch = GIT_FETCH_OPTIONS_INIT;

	g_repo = NULL;

	memset(&g_options, 0, sizeof(git_clone_options));
	g_options.version = GIT_CLONE_OPTIONS_VERSION;
	g_options.checkout_opts = dummy_opts;
	g_options.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	g_options.fetch_opts = dummy_fetch;
	g_options.fetch_opts.callbacks.certificate_check = ssl_cert;

	_remote_expectcontinue = cl_getenv("GITTEST_REMOTE_EXPECTCONTINUE");

	if (_remote_expectcontinue)
		git_libgit2_opts(GIT_OPT_ENABLE_HTTP_EXPECT_CONTINUE, 1);
}

void test_online_slow_clone__cleanup(void)
{
	if (g_repo) {
		git_repository_free(g_repo);
		g_repo = NULL;
	}
	cl_fixture_cleanup("./foo");

	git__free(_remote_expectcontinue);
}

void test_online_slow_clone__can_clone_with_https(void)
{
	g_options.fetch_opts.callbacks.certificate_check = ssl_cert;

	cl_git_pass(git_clone(&g_repo, SLOW_LIVE_REPO, "./foo", &g_options));
}

static int transferProgress(const git_indexer_progress *stats, void *payload) {
  GIT_UNUSED(stats);
  GIT_UNUSED(payload);
  return 0;
}

void test_online_slow_clone__can_clone_with_https_twice(void)
{
	g_options.fetch_opts.callbacks.transfer_progress = transferProgress;

	cl_git_pass(git_clone(&g_repo, SLOW_LIVE_REPO, "./foo", &g_options));

  git_repository_free(g_repo);
  g_repo = NULL;
  cl_fixture_cleanup("./foo");

  cl_git_pass(git_clone(&g_repo, SLOW_LIVE_REPO, "./foo", &g_options));
}
